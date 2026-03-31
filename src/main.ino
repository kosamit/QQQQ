/*
 * @Description: QQQQ with FreeRTOS
 * @Author: kosamit
 * @Date: 2025-10-05 09:34:58
 * @LastEditTime: 2025-10-23 15:00:00
 */

#include "globals.h"
#include "common/common_functions.h"
#include "debug/touch_info.h"
#include "screens/screens.h"
#include "bluetooth/bluetooth.h"
#include "midi/midi_handler.h"
#include "tasks/tasks.h"
#include "neotrellis/neotrellis_handler.h"
#include "webserver/webserver.h"
#include <SPI.h>
#include <SD.h>

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
BLEMIDI_CREATE_INSTANCE(DEVICE_NAME, MIDI);

void Arduino_IIC_Touch_Interrupt(void)
{
    CST226SE->IIC_Interrupt_Flag = true;
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

    // SD カード初期化
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (SD.begin(SD_CS, SPI, 40000000)) {
        Serial.println("SD card initialization successful");
        SD_Initialization_Flag = true;
    } else {
        Serial.println("SD card initialization failed");
        SD_Initialization_Flag = false;
    }

    // オーディオ I2S ピン設定
    audio.setPinout(MAX98357A_BCLK, MAX98357A_LRCLK, MAX98357A_DATA);
    Volume_Value = 10;
    audio.setVolume(Volume_Value); // 0...21、音量設定

    // BLE-MIDI 初期化（手動起動に変更）
    Serial.println("BLE-MIDI ready (manual start)");
    Serial.println("Use Bluetooth menu to start");

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

            // Webファイルマネージャー起動
            if (SD_Initialization_Flag) {
                startWebServer();
            }
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
    gfx->fillScreen(BLACK);

    // Initialize touch info
    Init_Touch_Info();

    // グリッドを作成（ドラムパッド画面用に準備）
    grid = new Grid4x4(gfx);
    grid->init(140, 11, 50, 50);
    grid->setLineThickness(2);
    grid->setGridLineColor(0xFFFF);
    grid->setActiveColor(0x07E0);
    grid->setInactiveColor(0x0000);
    grid->setTouchMode(TOUCH_MODE_TOGGLE);
    grid->setDefaultMidiNotes(60);

    // NeoTrellis 初期化
    if (initNeoTrellis()) {
        Serial.println("NeoTrellis connected - drum pad ready");
    } else {
        Serial.println("NeoTrellis not found - touch-only mode");
    }

    // メニュー画面を表示
    drawMenuScreen();

    Serial.println(String(DEVICE_NAME) + " initialized - Menu screen");

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
        displayTask, "DisplayTask", 8192, NULL, 4, &displayTaskHandle, 1);
    if (result != pdPASS) {
        Serial.println("ディスプレイタスクの作成に失敗しました");
        while(1);
    }

    // タッチ処理タスク (優先度: 高)
    result = xTaskCreatePinnedToCore(
        touchTask, "TouchTask", 4096, NULL, 3, &touchTaskHandle, 1);
    if (result != pdPASS) {
        Serial.println("タッチタスクの作成に失敗しました");
        while(1);
    }

    // 時計更新タスク (優先度: 低)
    result = xTaskCreatePinnedToCore(
        clockTask, "ClockTask", 4096, NULL, 1, &clockTaskHandle, 1);
    if (result != pdPASS) {
        Serial.println("時計タスクの作成に失敗しました");
        while(1);
    }

    // NeoTrellisポーリングタスク (優先度: 中 - 接続時のみ)
    if (neotrellisConnected) {
        result = xTaskCreatePinnedToCore(
            neotrellisTask, "NeoTrellisTask", 4096, NULL, 2, &neotrellisTaskHandle, 1);
        if (result != pdPASS) {
            Serial.println("NeoTrellisタスクの作成に失敗しました");
        } else {
            Serial.println("NeoTrellisタスク開始");
        }
    }

    Serial.println("FreeRTOS タスクが正常に開始されました");
}

void loop()
{
    // オーディオ再生ループ（必須）
    audio.loop();

    // Webサーバー処理
    handleWebServer();

    // BLE-MIDIのreadを定期的に呼び出して接続を維持
    if (bleAdvertising) {
        MIDI.read();
    }
}
