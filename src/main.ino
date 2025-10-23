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
    bus, LCD_RST /* RST */, 0 /* rotation */, true /* IPS */,
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

// グリッドオブジェクト
Grid4x4* grid;

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
    50,                // x: グリッドと同じX位置
    320,               // y: グリッドの下
    240,               // width: グリッドと同じ幅
    40,                // height: ボタンの高さ
    "Mode: TOGGLE",    // label_toggle
    "Mode: HOLD"       // label_hold
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
    grid = new Grid4x4(gfx);
    grid->init(50, 50, 60, 60);  // x=50, y=50から開始、セルサイズ60x60
    
    // グリッドの設定
    grid->setLineThickness(2);
    grid->setGridLineColor(0xFFFF);      // 白い線
    grid->setActiveColor(0x07E0);        // 緑色（アクティブ時）
    grid->setInactiveColor(0x0000);      // 黒色（非アクティブ時）
    grid->setTouchMode(TOUCH_MODE_TOGGLE); // タッチで切り替えモード
    
    // グリッドを描画
    grid->draw();
    
    // モード切り替えボタンを描画
    drawModeButton();
    
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
        Serial.println("タッチタスクの作成に失敗しました");
        while(1);
    }
    
    // ディスプレイ更新タスク (優先度: 中)
    result = xTaskCreatePinnedToCore(
        displayTask,         // タスク関数
        "DisplayTask",       // タスク名
        8192,                // スタックサイズ (大きめ)
        NULL,                // パラメータ
        2,                   // 優先度 (中)
        &displayTaskHandle,  // タスクハンドル
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

// タッチ処理タスク（割り込み駆動・マルチタッチ対応）
void touchTask(void* parameter)
{
    Serial.println("タッチタスク開始（割り込み駆動・マルチタッチ対応）");
    
    while (true) {
        // 割り込みからの通知を待機
        uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if (notificationValue > 0) {
            // 少し待ってから読み取り（デバウンス）
            vTaskDelay(pdMS_TO_TICKS(20));
            
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
            
            // デバッグ出力
            if (event.finger_count > 0) {
                Serial.printf("マルチタッチ: %d本\n", event.finger_count);
                for (int i = 0; i < event.finger_count; i++) {
                    Serial.printf("  指%d: X=%d, Y=%d\n", i+1, event.x[i], event.y[i]);
                }
            } else {
                Serial.println("タッチ終了");
            }
            
            // キューにイベントを送信
            if (xQueueSend(touchEventQueue, &event, 0) != pdTRUE) {
                Serial.println("タッチイベントキューが満杯です");
            }
            
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
        // キューからタッチイベントを受信 (最大100ms待機)
        if (xQueueReceive(touchEventQueue, &event, pdMS_TO_TICKS(100)) == pdTRUE) {
            Serial.printf("ディスプレイタスク: %d本の指を検出\n", event.finger_count);
            
            // ディスプレイミューテックスを取得
            if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                bool buttonHandled = false;
                bool currentButtonTouched = false;
                
                // モード切り替えボタンのタッチを判定
                if (event.finger_count > 0 && 
                    isTouchInButton(event.x[0], event.y[0], modeButton)) {
                    currentButtonTouched = true;
                    
                    // 新しくボタンがタッチされた（前回はタッチされていなかった）場合のみ切り替え
                    if (!lastButtonTouched) {
                        // タッチモードを切り替え
                        if (currentTouchMode == TOUCH_MODE_TOGGLE) {
                            currentTouchMode = TOUCH_MODE_HOLD;
                            Serial.println("モード変更: HOLD");
                        } else {
                            currentTouchMode = TOUCH_MODE_TOGGLE;
                            Serial.println("モード変更: TOGGLE");
                        }
                        
                        grid->setTouchMode(currentTouchMode);
                        drawModeButton();
                    } else {
                        Serial.println("ボタン長押し中 - 何もしない");
                    }
                    
                    buttonHandled = true;
                }
                
                // ボタンのタッチ状態を記録
                lastButtonTouched = currentButtonTouched;
                
                // ボタンが押されていなければ、グリッドでマルチタッチを処理
                if (!buttonHandled) {
                    grid->handleMultiTouch(event.x, event.y, event.finger_count);
                    
                    // 変更されたセルを再描画
                    grid->redrawChangedCells();
                    
                    // アクティブなセルの数を表示
                    int16_t activeCount = grid->getActiveCellCount();
                    Serial.printf("アクティブセル数: %d\n", activeCount);
                }
                
                // ミューテックスを解放
                xSemaphoreGive(displayMutex);
            } else {
                Serial.println("ディスプレイミューテックスの取得に失敗しました");
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
        // ディスプレイミューテックスを取得
        if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            // 時計情報を更新
            GFX_Print_Time_Info_Loop();
            
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

void loop()
{
    // FreeRTOSタスクが全てを処理するため、loop()は空にする
    // ただし、完全に削除はできないため、短い遅延を入れる
    vTaskDelay(pdMS_TO_TICKS(1000));
}
