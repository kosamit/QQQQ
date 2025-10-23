/*
 * @Description: Qurospad with FreeRTOS
 * @Author: kosamit
 * @Date: 2025-10-05 09:34:58
 * @LastEditTime: 2025-10-23 15:00:00
 */

#include "common/common_config.h"
#include "common/common_functions.h"
#include "debug/touch_info.h"
#include "lib/grid.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>
#include <esp_bt.h>
#include <esp_bt_main.h>

// File Download URL Definition
const char *fileDownloadUrl = "https://freetyst.nf.migu.cn/public/product9th/product45/2022/05/0716/2018%E5%B9%B409%E6%9C%8812%E6%97%A510%E7%82%B943%E5%88%86%E7%B4%A7%E6%80%A5%E5%86%85%E5%AE%B9%E5%87%86%E5%85%A5%E5%8D%8E%E7%BA%B3179%E9%A6%96/%E6%A0%87%E6%B8%85%E9%AB%98%E6%B8%85/MP3_128_16_Stero/6005751EPFG164228.mp3?channelid=02&msisdn=d43a7dcc-8498-461b-ba22-3205e9b6aa82&Tim=1728484238063&Key=0442fa065dacda7c";

// Global Variables Definition
bool Wifi_Connection_Flag = true;
bool SD_Initialization_Flag = false;
bool SD_File_Download_Check_Flag = false;

char IIS_Read_Buff[100];

size_t CycleTime = 0;

uint8_t Image_Flag = 0;
int8_t Volume_Value = 0;
uint8_t Music_Start_Playing_Count = 0;
bool Music_Start_Playing_Flag = false;

int8_t Strength_Value = 0;

bool Skip_Current_Test = false;

Audio audio(false, 3, I2S_NUM_1);

Arduino_DataBus *bus = new Arduino_HWSPI(
    LCD_DC /* DC */, LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_MOSI /* MOSI */, LCD_MISO /* MISO */);

Arduino_GFX *gfx = new Arduino_ST7796(
    bus, LCD_RST /* RST */, 3 /* rotation */, true /* IPS */,
    LCD_WIDTH /* width */, LCD_HEIGHT /* height */,
    49 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::shared_ptr<Arduino_IIS_DriveBus> IIS_Bus =
    std::make_shared<Arduino_HWIIS>(I2S_NUM_0, MSM261_BCLK, MSM261_WS, MSM261_DATA);

std::unique_ptr<Arduino_IIS> IIS(new Arduino_MEMS(IIS_Bus));

std::unique_ptr<Arduino_IIC> CST226SE(new Arduino_CST2xxSE(IIC_Bus, CST226SE_DEVICE_ADDRESS,
                                                           TOUCH_RST, TOUCH_INT, Arduino_IIC_Touch_Interrupt));

std::unique_ptr<Arduino_IIC> SY6970(new Arduino_SY6970(IIC_Bus, SY6970_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, DRIVEBUS_DEFAULT_VALUE));

std::unique_ptr<Arduino_IIC> PCF85063(new Arduino_PCF85063(IIC_Bus, PCF85063_DEVICE_ADDRESS,
                                                           DRIVEBUS_DEFAULT_VALUE, PCF85063_INT, Arduino_IIC_RTC_Interrupt));

// BLE-MIDI インスタンス
BLEMIDI_CREATE_INSTANCE("Qurospad", MIDI);

// グリッドオブジェクト
Grid4x4* grid;

// 前回のセル状態を保持（MIDI送信用）
bool prevCellState[GRID_ROWS * GRID_COLS] = {false};

// Bluetooth接続状態
bool bleConnected = false;
bool bleAdvertising = false;  // アドバタイズ中かどうか
unsigned long lastBleStatusUpdate = 0;

// モード切り替えボタンの定義
struct ModeButton {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    const char* label_toggle;
    const char* label_hold;
};

ModeButton modeButton = {
    10,                // x: 左上
    10,                // y: 左上
    120,               // width: ボタンの幅
    40,                // height: ボタンの高さ
    "Mode: TOGGLE",    // label_toggle
    "Mode: HOLD"       // label_hold
};

// Bluetoothトグルボタンの定義
struct BluetoothButton {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
};

BluetoothButton bleButton = {
    350,               // x: 右側
    172,               // y: 下側（222-40-10=172）
    120,               // width
    40                 // height
};

// 現在のタッチモード
TouchMode currentTouchMode = TOUCH_MODE_TOGGLE;

// FreeRTOS オブジェクト
TaskHandle_t touchTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t clockTaskHandle = NULL;
QueueHandle_t touchEventQueue = NULL;
SemaphoreHandle_t displayMutex = NULL;

// タッチイベント構造体（マルチタッチ対応）
struct TouchEvent {
    uint8_t finger_count;      // タッチしている指の数
    int16_t x[5];              // 各タッチポイントのX座標
    int16_t y[5];              // 各タッチポイントのY座標
};

// FreeRTOS タスク関数の前方宣言
void touchTask(void* parameter);
void displayTask(void* parameter);
void clockTask(void* parameter);

// ボタン関連関数の前方宣言
void drawModeButton();
bool isTouchInButton(int16_t touchX, int16_t touchY, const ModeButton& btn);

// MIDI関連関数の前方宣言
void sendMidiNotesForChangedCells();
void drawBluetoothStatus();
void onBLEConnected();
void onBLEDisconnected();
void toggleBluetooth();
void startBluetooth();
void stopBluetooth();

void Arduino_IIC_Touch_Interrupt(void)
{
    CST226SE->IIC_Interrupt_Flag = true;
    // タッチタスクに通知
    if (touchTaskHandle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(touchTaskHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void Arduino_IIC_RTC_Interrupt(void)
{
    PCF85063->IIC_Interrupt_Flag = true;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");
    Serial.println("[T-Display-S3-Pro-MVSRBoard_" + (String)BOARD_VERSION "][" + (String)SOFTWARE_NAME +
                   "]_firmware_" + (String)SOFTWARE_LASTEDITTIME);

    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    pinMode(MSM261_EN, OUTPUT);
    digitalWrite(MSM261_EN, HIGH);

    pinMode(MAX98357A_SD_MODE, OUTPUT);
    digitalWrite(MAX98357A_SD_MODE, HIGH);

    ledcAttachPin(LCD_BL, 1);
    ledcSetup(1, 2000, 8);
    ledcWrite(1, 255);

    ledcAttachPin(VIBRATINO_MOTOR_PWM, 2);
    ledcSetup(2, 12000, 8);
    ledcWrite(2, 0);

    if (SY6970->begin() == false)
    {
        Serial.println("SY6970 initialization fail");
        delay(2000);
    }
    else
    {
        Serial.println("SY6970 initialization successfully");
    }

    // ADC測定機能を有効化
    if (SY6970->IIC_Write_Device_State(SY6970->Arduino_IIC_Power::Device::POWER_DEVICE_ADC_MEASURE,
                                       SY6970->Arduino_IIC_Power::Device_State::POWER_DEVICE_ON) == false)
    {
        Serial.println("SY6970 ADC Measure ON fail");
        delay(2000);
    }
    else
    {
        Serial.println("SY6970 ADC Measure ON successfully");
    }

    // ウォッチドッグタイマーのフィード機能を無効化
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_WATCHDOG_TIMER, 0);
    // 熱調節しきい値を60度に設定
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_THERMAL_REGULATION_THRESHOLD, 60);
    // 充電目標電圧を4224mVに設定
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_CHARGING_TARGET_VOLTAGE_LIMIT, 4224);
    // 最小システム電圧制限を3600mVに設定
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_MINIMUM_SYSTEM_VOLTAGE_LIMIT, 3600);
    // OTG電圧を5062mVに設定
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_OTG_VOLTAGE_LIMIT, 5062);
    // 入力電流制限を2100mAに設定
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_INPUT_CURRENT_LIMIT, 2100);
    // 高速充電電流制限を2112mAに設定
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_FAST_CHARGING_CURRENT_LIMIT, 2112);
    // プリ充電電流制限を192mAに設定
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_PRECHARGE_CHARGING_CURRENT_LIMIT, 192);
    // 終端充電電流制限を320mAに設定
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_TERMINATION_CHARGING_CURRENT_LIMIT, 320);
    // OTG電流制限を500mAに設定
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_OTG_CHARGING_LIMIT, 500);

    if (CST226SE->begin() == false)
    {
        Serial.println("CST226SE initialization fail");
        delay(2000);
    }
    else
    {
        Serial.println("CST226SE initialization successfully");
    }

    if (PCF85063->begin() == false)
    {
        Serial.println("PCF85063 initialization fail");
        delay(2000);
    }
    else
    {
        Serial.println("PCF85063 initialization successfully");
    }

    // 時間形式を24時間制に設定
    PCF85063->IIC_Write_Device_State(PCF85063->Arduino_IIC_RTC::Device::RTC_CLOCK_TIME_FORMAT,
                                     PCF85063->Arduino_IIC_RTC::Device_Mode::RTC_CLOCK_TIME_FORMAT_24);

    // クロック出力を無効化
    PCF85063->IIC_Write_Device_State(PCF85063->Arduino_IIC_RTC::Device::RTC_CLOCK_OUTPUT_VALUE,
                                     PCF85063->Arduino_IIC_RTC::Device_Mode::RTC_CLOCK_OUTPUT_OFF);

    // RTCを有効化
    PCF85063->IIC_Write_Device_State(PCF85063->Arduino_IIC_RTC::Device::RTC_CLOCK_RTC,
                                     PCF85063->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

    Volume_Value = 3;
    audio.setVolume(Volume_Value); // 0...21、音量設定

    // BLE-MIDI 初期化（自動起動）
    Serial.println("Initializing BLE-MIDI...");
    startBluetooth();

    // WiFi接続を開始（設定で有効化されている場合のみ）
    if (ENABLE_WIFI_CONNECTION) {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        Serial.print("WiFi接続中");
        unsigned long wifi_start_time = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - wifi_start_time < WIFI_CONNECT_WAIT_MAX) {
            delay(500);
            Serial.print(".");
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println();
            Serial.println("WiFi接続成功");
            Serial.print("IPアドレス: ");
            Serial.println(WiFi.localIP());
            
            // NTP時間同期設定
            configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
            Wifi_Connection_Flag = true;
        } else {
            Serial.println();
            Serial.println("WiFi接続失敗");
            Wifi_Connection_Flag = false;
        }
    } else {
        Serial.println("WiFi接続は無効化されています - RTC時刻を使用します");
        Wifi_Connection_Flag = false;
    }

    gfx->begin();
    gfx->setTextSize(1);
    gfx->fillScreen(WHITE);
    
    // Initialize touch info
    Init_Touch_Info();
    
    // グリッドを作成（画面中央に配置）
    // 画面サイズ: 480x222 (rotation=3で回転後)
    // セルサイズ: 50x50、グリッド全体: 200x200
    // 中央配置: X=(480-200)/2=140, Y=(222-200)/2=11
    grid = new Grid4x4(gfx);
    grid->init(140, 11, 50, 50);  // x=140, y=11から開始、セルサイズ50x50
    
    // グリッドの設定
    grid->setLineThickness(2);
    grid->setGridLineColor(0xFFFF);      // 白い線
    grid->setActiveColor(0x07E0);        // 緑色（アクティブ時）
    grid->setInactiveColor(0x0000);      // 黒色（非アクティブ時）
    grid->setTouchMode(TOUCH_MODE_TOGGLE); // タッチで切り替えモード
    
    // MIDIノート番号を設定（C4=60から開始）
    grid->setDefaultMidiNotes(60);
    
    // グリッドを描画
    grid->draw();
    
    // モード切り替えボタンを描画
    drawModeButton();
    
    // Bluetooth接続状態を描画
    drawBluetoothStatus();
    
    Serial.println("4x4 Grid initialized");
    
    // FreeRTOS オブジェクトの初期化
    touchEventQueue = xQueueCreate(10, sizeof(TouchEvent));
    if (touchEventQueue == NULL) {
        Serial.println("タッチイベントキューの作成に失敗しました");
        while(1);
    }
    
    displayMutex = xSemaphoreCreateMutex();
    if (displayMutex == NULL) {
        Serial.println("ディスプレイミューテックスの作成に失敗しました");
        while(1);
    }
    
    // FreeRTOS タスクの作成
    BaseType_t result;
    
    // ディスプレイ更新タスク (優先度: 最高 - 画面更新を最優先)
    result = xTaskCreatePinnedToCore(
        displayTask,         // タスク関数
        "DisplayTask",       // タスク名
        8192,                // スタックサイズ (大きめ)
        NULL,                // パラメータ
        4,                   // 優先度 (最高)
        &displayTaskHandle,  // タスクハンドル
        1                    // コア1で実行
    );
    if (result != pdPASS) {
        Serial.println("ディスプレイタスクの作成に失敗しました");
        while(1);
    }
    
    // タッチ処理タスク (優先度: 高)
    result = xTaskCreatePinnedToCore(
        touchTask,           // タスク関数
        "TouchTask",         // タスク名
        4096,                // スタックサイズ
        NULL,                // パラメータ
        3,                   // 優先度 (高)
        &touchTaskHandle,    // タスクハンドル
        1                    // コア1で実行
    );
    if (result != pdPASS) {
        Serial.println("ディスプレイタスクの作成に失敗しました");
        while(1);
    }
    
    // 時計更新タスク (優先度: 低)
    result = xTaskCreatePinnedToCore(
        clockTask,           // タスク関数
        "ClockTask",         // タスク名
        4096,                // スタックサイズ
        NULL,                // パラメータ
        1,                   // 優先度 (低)
        &clockTaskHandle,    // タスクハンドル
        1                    // コア1で実行
    );
    if (result != pdPASS) {
        Serial.println("時計タスクの作成に失敗しました");
        while(1);
    }
    
    Serial.println("FreeRTOS タスクが正常に開始されました");
}

// タッチ処理タスク（割り込み駆動・マルチタッチ対応・高速化版）
void touchTask(void* parameter)
{
    Serial.println("タッチタスク開始（高速化モード）");
    
    while (true) {
        // 割り込みからの通知を待機
        uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if (notificationValue > 0) {
            // 少し待ってから読み取り（デバウンス: 3ms = 最高速化）
            vTaskDelay(pdMS_TO_TICKS(3));
            
            // タッチ情報を更新
            Update_Touch_Info();
            
            // マルチタッチイベントを作成
            TouchEvent event;
            event.finger_count = global_touch_info.fingers_number;
            
            // 全てのタッチポイントをコピー
            for (int i = 0; i < 5; i++) {
                event.x[i] = global_touch_info.touch_x[i];
                event.y[i] = global_touch_info.touch_y[i];
            }
            
            // キューにイベントを送信
            xQueueSend(touchEventQueue, &event, 0);
            
            // 割り込みフラグをクリア
            CST226SE->IIC_Interrupt_Flag = false;
        }
    }
}

// ディスプレイ更新タスク（マルチタッチ対応）
void displayTask(void* parameter)
{
    Serial.println("ディスプレイタスク開始（マルチタッチ対応）");
    
    TouchEvent event;
    static bool lastButtonTouched = false;  // 前回ボタンがタッチされていたか
    
    while (true) {
        // キューからタッチイベントを受信 (最大1ms待機 = 最高速ポーリング)
        if (xQueueReceive(touchEventQueue, &event, pdMS_TO_TICKS(1)) == pdTRUE) {
            // ディスプレイミューテックスを取得（即座に取得）
            if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                bool buttonHandled = false;
                bool currentButtonTouched = false;
                
                // モード切り替えボタンのタッチを判定
                if (event.finger_count > 0 && 
                    isTouchInButton(event.x[0], event.y[0], modeButton)) {
                    currentButtonTouched = true;
                    
                    // 新しくボタンがタッチされた場合のみ切り替え
                    if (!lastButtonTouched) {
                        // タッチモードを切り替え
                        if (currentTouchMode == TOUCH_MODE_TOGGLE) {
                            currentTouchMode = TOUCH_MODE_HOLD;
                        } else {
                            currentTouchMode = TOUCH_MODE_TOGGLE;
                        }
                        
                        grid->setTouchMode(currentTouchMode);
                        drawModeButton();
                    }
                    
                    buttonHandled = true;
                }
                
                // Bluetoothボタンのタッチを判定
                if (event.finger_count > 0 && !buttonHandled) {
                    int16_t touchX = event.x[0];
                    int16_t touchY = event.y[0];
                    if (touchX >= bleButton.x && touchX < bleButton.x + bleButton.width &&
                        touchY >= bleButton.y && touchY < bleButton.y + bleButton.height) {
                        
                        if (!lastButtonTouched) {
                            toggleBluetooth();
                            drawBluetoothStatus();
                        }
                        currentButtonTouched = true;
                        buttonHandled = true;
                    }
                }
                
                // ボタンのタッチ状態を記録
                lastButtonTouched = currentButtonTouched;
                
                // ボタンが押されていなければ、グリッドでマルチタッチを処理
                if (!buttonHandled) {
                    grid->handleMultiTouch(event.x, event.y, event.finger_count);
                    
                    // HOLDモードの場合のみMIDIノートを送信
                    if (currentTouchMode == TOUCH_MODE_HOLD) {
                        sendMidiNotesForChangedCells();
                    }
                    
                    // 変更されたセルを再描画
                    grid->redrawChangedCells();
                }
                
                // ミューテックスを解放
                xSemaphoreGive(displayMutex);
            }
        }
    }
}

// 時計更新タスク
void clockTask(void* parameter)
{
    Serial.println("時計タスク開始");
    
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t updateInterval = pdMS_TO_TICKS(1000); // 1秒間隔
    
    while (true) {
        // ディスプレイミューテックスを取得（短時間待機）
        if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            // 時計情報を更新
            GFX_Print_Time_Info_Loop();
            
            // Bluetooth接続状態を定期的に更新
            unsigned long currentMillis = millis();
            if (currentMillis - lastBleStatusUpdate > 1000) {
                drawBluetoothStatus();
                lastBleStatusUpdate = currentMillis;
            }
            
            // ミューテックスを解放
            xSemaphoreGive(displayMutex);
        }
        
        // 次の更新まで待機
        vTaskDelayUntil(&lastWakeTime, updateInterval);
    }
}

// モード切り替えボタンを描画
void drawModeButton() {
    // ボタンの背景を描画
    uint16_t bgColor = (currentTouchMode == TOUCH_MODE_TOGGLE) ? 0x2104 : 0x8C51; // モードに応じて色を変える
    gfx->fillRect(modeButton.x, modeButton.y, modeButton.width, modeButton.height, bgColor);
    
    // ボタンの枠を描画
    gfx->drawRect(modeButton.x, modeButton.y, modeButton.width, modeButton.height, WHITE);
    gfx->drawRect(modeButton.x + 1, modeButton.y + 1, modeButton.width - 2, modeButton.height - 2, WHITE);
    
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    
    // テキストを中央に配置
    const char* label = (currentTouchMode == TOUCH_MODE_TOGGLE) ? modeButton.label_toggle : modeButton.label_hold;
    int16_t textWidth = strlen(label) * 12;  // 概算の幅
    int16_t textX = modeButton.x + (modeButton.width - textWidth) / 2;
    int16_t textY = modeButton.y + (modeButton.height - 16) / 2;
    
    gfx->setCursor(textX, textY);
    gfx->print(label);
}

// ボタンがタッチされたか判定
bool isTouchInButton(int16_t touchX, int16_t touchY, const ModeButton& btn) {
    return (touchX >= btn.x && touchX < btn.x + btn.width &&
            touchY >= btn.y && touchY < btn.y + btn.height);
}

// MIDI Note On/Off送信関数（ホールドモード専用）
void sendMidiNotesForChangedCells() {
    for (int16_t row = 0; row < GRID_ROWS; row++) {
        for (int16_t col = 0; col < GRID_COLS; col++) {
            int16_t index = row * GRID_COLS + col;
            bool currentState = grid->isCellActive(row, col);
            
            // 状態が変化した場合のみMIDI送信
            if (currentState != prevCellState[index]) {
                uint8_t note = grid->getCellMidiNote(row, col);
                uint8_t velocity = 100;  // ベロシティは100に固定
                
                if (currentState) {
                    // Note On送信（BLE-MIDI）
                    MIDI.sendNoteOn(note, velocity, 1);  // チャンネル1
                    Serial.printf("BLE-MIDI Note On: %d (Row:%d, Col:%d)\n", note, row, col);
                } else {
                    // Note Off送信（BLE-MIDI）
                    MIDI.sendNoteOff(note, 0, 1);  // チャンネル1
                    Serial.printf("BLE-MIDI Note Off: %d (Row:%d, Col:%d)\n", note, row, col);
                }
                
                // 前回の状態を更新
                prevCellState[index] = currentState;
            }
        }
    }
}

// Bluetooth接続状態を描画
void drawBluetoothStatus() {
    // Bluetooth状態表示エリア（右上）
    int16_t statusX = bleButton.x;
    int16_t statusY = bleButton.y;
    int16_t statusWidth = bleButton.width;
    int16_t statusHeight = bleButton.height;
    
    // 背景色を決定
    uint16_t bgColor;
    const char* statusText;
    
    if (bleConnected) {
        bgColor = 0x07E0;      // 緑: 接続済み
        statusText = "CONNECTED";
    } else if (bleAdvertising) {
        bgColor = 0xFFE0;      // 黄色: 待機中
        statusText = "WAITING";
    } else {
        bgColor = 0xF800;      // 赤: オフ
        statusText = "BLE OFF";
    }
    
    // 背景を描画
    gfx->fillRect(statusX, statusY, statusWidth, statusHeight, bgColor);
    
    // 枠を描画
    gfx->drawRect(statusX, statusY, statusWidth, statusHeight, WHITE);
    gfx->drawRect(statusX + 1, statusY + 1, statusWidth - 2, statusHeight - 2, WHITE);
    
    // テキストを描画
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);
    
    int16_t textWidth = strlen(statusText) * 6;
    int16_t textX = statusX + (statusWidth - textWidth) / 2;
    int16_t textY = statusY + (statusHeight - 8) / 2;
    
    gfx->setCursor(textX, textY);
    gfx->print(statusText);
    
    // デバイス名を表示（画面下部）
    if (bleAdvertising && !bleConnected) {
        gfx->fillRect(0, 200, 480, 22, BLACK);  // 背景をクリア
        gfx->setTextSize(2);
        gfx->setTextColor(WHITE);
        gfx->setCursor(10, 200);
        gfx->print("BLE: Qurospad");
        
        gfx->setTextSize(1);
        gfx->setCursor(350, 200);
        gfx->print("Tap to stop");
    }
}

// Bluetooth開始
void startBluetooth() {
    if (!bleAdvertising) {
        Serial.println("========================================");
        Serial.println("Starting BLE-MIDI...");
        
        // ESP32のBLEを初期化（停止されていた場合）
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE) {
            esp_bt_controller_init(&bt_cfg);
            esp_bt_controller_enable(ESP_BT_MODE_BLE);
            esp_bluedroid_init();
            esp_bluedroid_enable();
            Serial.println("BLE hardware initialized");
        }
        
        // 少し待機
        delay(100);
        
        // BLE接続コールバックを設定（begin前に設定）
        BLEMIDI.setHandleConnected(onBLEConnected);
        BLEMIDI.setHandleDisconnected(onBLEDisconnected);
        
        // BLE-MIDI開始
        MIDI.begin(MIDI_CHANNEL_OMNI);
        
        bleAdvertising = true;
        
        Serial.println("BLE-MIDI started successfully");
        Serial.println("Device name: Qurospad");
        Serial.println("Status: Advertising...");
        Serial.println("========================================");
    } else {
        Serial.println("BLE already running");
    }
}

// Bluetooth停止
void stopBluetooth() {
    if (bleAdvertising) {
        Serial.println("========================================");
        Serial.println("Stopping BLE-MIDI...");
        
        // BLE切断
        bleConnected = false;
        bleAdvertising = false;
        
        // ESP32のBLEを完全に停止
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        
        Serial.println("BLE-MIDI stopped completely");
        Serial.println("BLE hardware disabled");
        Serial.println("========================================");
    }
}

// Bluetoothトグル
void toggleBluetooth() {
    if (bleAdvertising) {
        stopBluetooth();
    } else {
        startBluetooth();
    }
}

// BLE接続時のコールバック
void onBLEConnected() {
    Serial.println("========================================");
    Serial.println("✓ BLE CONNECTED!");
    Serial.println("Device is now connected");
    Serial.println("========================================");
    
    bleConnected = true;
    
    if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        drawBluetoothStatus();
        xSemaphoreGive(displayMutex);
    }
}

// BLE切断時のコールバック
void onBLEDisconnected() {
    Serial.println("========================================");
    Serial.println("✗ BLE DISCONNECTED");
    Serial.println("========================================");
    
    bleConnected = false;
    
    // 再接続のために少し待機してから再度アドバタイズを開始
    if (bleAdvertising) {
        Serial.println("Preparing for reconnection...");
        delay(1000);  // 1秒待機
        
        // BLE-MIDIを再初期化して再接続可能にする
        BLEMIDI.setHandleConnected(onBLEConnected);
        BLEMIDI.setHandleDisconnected(onBLEDisconnected);
        MIDI.begin(MIDI_CHANNEL_OMNI);
        
        Serial.println("========================================");
        Serial.println("Ready for reconnection");
        Serial.println("Status: Advertising...");
        Serial.println("========================================");
    }
    
    if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        drawBluetoothStatus();
        xSemaphoreGive(displayMutex);
    }
}

void loop()
{
    // BLE-MIDIのreadを定期的に呼び出して接続を維持
    // これは接続状態の監視とイベント処理に必要
    if (bleAdvertising) {
        MIDI.read();
    }
    
    // FreeRTOSタスクが全てを処理するため、短い遅延を入れる
    delay(10);
}
