/*
 * @Description: Qurospad
 * @Author: kosamit
 * @Date: 2025-10-05 09:34:58
 * @LastEditTime: 2025-10-05 09:35:15
 */

#include "common/common_config.h"
#include "common/common_functions.h"
#include "debug/touch_info.h"
#include "lib/grid.h"

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

void Arduino_IIC_Touch_Interrupt(void)
{
    CST226SE->IIC_Interrupt_Flag = true;
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
    
    Serial.println("4x4 Grid initialized");
}

void loop()
{
    // 時計表示
    if (millis() > CycleTime)
    {
        GFX_Print_Time_Info_Loop();
        CycleTime = millis() + 1000;
    }

    // タッチ情報を更新
    Update_Touch_Info();
    
    // タッチするたびの更新処理
    if (global_touch_info.has_changed) {
        // タッチ情報を表示
        // Print_Touch_Info();
        
        // グリッドでタッチを処理
        if (global_touch_info.fingers_number > 0) {
            int16_t touchX = global_touch_info.touch_x[0];
            int16_t touchY = global_touch_info.touch_y[0];
            bool isPressed = global_touch_info.is_touched;
            
            if (grid->handleTouch(touchX, touchY, isPressed)) {
                // タッチがグリッド内で処理された
                Serial.println("Grid touched!");
                
                // アクティブなセルの数を表示
                int16_t activeCount = grid->getActiveCellCount();
                Serial.print("Active cells: ");
                Serial.println(activeCount);
                
                // グリッドを再描画
                grid->redraw();
            }
        }
    }

    // 4x4のパッドを表示
    
}
