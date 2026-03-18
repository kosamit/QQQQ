/*
 * @Description: Common Functions Implementation for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "common_functions.h"

void Vibration_Start(void)
{
    ledcWrite(2, 255);
    delay(10);
    ledcWrite(2, 0);
}

void Skip_Test_Loop()
{
    uint8_t fingers_number = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

    if (fingers_number > 0)
    {
        int32_t touch_x = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
        int32_t touch_y = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

        if (touch_x > 40 && touch_x < 140 &&
            touch_y > LCD_HEIGHT - (LCD_HEIGHT / 3) && touch_y < LCD_HEIGHT - (LCD_HEIGHT / 3) + 40)
        {
            Vibration_Start();
            Skip_Current_Test = true;
        }
    }
}

void GFX_Print_1()
{
    gfx->fillRect(20, LCD_HEIGHT - (LCD_HEIGHT / 4), 80, 40, ORANGE);
    gfx->drawRect(20, LCD_HEIGHT - (LCD_HEIGHT / 4), 80, 40, PURPLE);
    gfx->fillRect(120, LCD_HEIGHT - (LCD_HEIGHT / 4), 80, 40, PURPLE);
    gfx->drawRect(120, LCD_HEIGHT - (LCD_HEIGHT / 4), 80, 40, ORANGE);
    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);
    gfx->setCursor(35, LCD_HEIGHT - (LCD_HEIGHT / 4) + 15);
    gfx->printf("Try Again");
    gfx->setCursor(135, LCD_HEIGHT - (LCD_HEIGHT / 4) + 15);
    gfx->printf("Next Test");
}

void GFX_Print_2()
{
    gfx->fillRect(40, LCD_HEIGHT - (LCD_HEIGHT / 3), 140, 40, RED);
    gfx->drawRect(40, LCD_HEIGHT - (LCD_HEIGHT / 3), 140, 40, CYAN);

    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);
    gfx->setCursor(60, LCD_HEIGHT - (LCD_HEIGHT / 3) + 15);
    gfx->printf("Skip Current Test");
}

void GFX_Print_TEST(String s)
{
    Skip_Current_Test = false;

    gfx->fillScreen(WHITE);
    gfx->setCursor(LCD_WIDTH / 4 + 20, LCD_HEIGHT / 4);
    gfx->setTextSize(3);
    gfx->setTextColor(PALERED);
    gfx->printf("TEST");

    gfx->setCursor(20, LCD_HEIGHT / 4 + 40);
    gfx->setTextSize(2);
    gfx->setTextColor(BLACK);
    gfx->print(s);

    GFX_Print_2();

    gfx->setCursor(LCD_WIDTH / 2 - 17, LCD_HEIGHT / 2);
    gfx->setTextSize(4);
    gfx->setTextColor(RED);
    gfx->printf("3");
    delay(300);
    gfx->fillRect(LCD_WIDTH / 2 - 17, LCD_HEIGHT / 2, LCD_WIDTH / 2 + 20, 20, WHITE);
    gfx->setCursor(LCD_WIDTH / 2 - 17, LCD_HEIGHT / 2);
    gfx->printf("2");
    for (int i = 0; i < 100; i++)
    {
        Skip_Test_Loop();
        delay(10);

        if (Skip_Current_Test == true)
        {
            break;
        }
    }
    gfx->fillRect(LCD_WIDTH / 2 - 17, LCD_HEIGHT / 2, LCD_WIDTH / 2 + 20, 20, WHITE);
    gfx->setCursor(LCD_WIDTH / 2 - 17, LCD_HEIGHT / 2);
    gfx->printf("1");
    for (int i = 0; i < 100; i++)
    {
        Skip_Test_Loop();
        delay(10);

        if (Skip_Current_Test == true)
        {
            break;
        }
    }
}

void GFX_Print_FINISH()
{
    gfx->setCursor(LCD_WIDTH / 4, LCD_HEIGHT / 4);
    gfx->setTextSize(3);
    gfx->setTextColor(ORANGE);
    gfx->printf("FINISH");
}

void GFX_Print_START()
{
    gfx->setCursor(150, 220);
    gfx->setTextSize(6);
    gfx->setTextColor(RED);
    gfx->printf("START");
}

void GFX_Print_Time_Info_Loop()
{
    // 右上の時間表示エリアをクリア（小さな矩形）
    // rotation=3の場合、画面は480x222になる
    gfx->fillRect(LCD_HEIGHT - 80, 10, 70, 20, WHITE);
    gfx->drawRect(LCD_HEIGHT - 80, 10, 70, 20, BLACK);

    if (Wifi_Connection_Flag == true)
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo, 1000))
        {
            Serial.println("Failed to obtain time");
            gfx->setCursor(LCD_HEIGHT - 75, 15);
            gfx->setTextColor(RED);
            gfx->setTextSize(1);
            gfx->print("Error");
            return;
        }
        // hh:mm:ss形式で右上に表示
        gfx->setCursor(LCD_HEIGHT - 75, 15);
        gfx->setTextColor(BLACK);
        gfx->setTextSize(1);
        gfx->print(&timeinfo, "%H:%M:%S");
    }
    else
    {
        // WiFi接続がない場合はRTC時刻を使用
        uint8_t hour = PCF85063->IIC_Read_Device_Value(PCF85063->Arduino_IIC_RTC::Value_Information::RTC_HOURS_DATA);
        uint8_t minute = PCF85063->IIC_Read_Device_Value(PCF85063->Arduino_IIC_RTC::Value_Information::RTC_MINUTES_DATA);
        uint8_t second = PCF85063->IIC_Read_Device_Value(PCF85063->Arduino_IIC_RTC::Value_Information::RTC_SECONDS_DATA);
        
        // hh:mm:ss形式で右上に表示
        gfx->setCursor(LCD_HEIGHT - 75, 15);
        gfx->setTextColor(BLACK);
        gfx->setTextSize(1);
        gfx->printf("%02d:%02d:%02d", hour, minute, second);
    }
}
