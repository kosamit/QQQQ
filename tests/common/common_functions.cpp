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
    delay(150);
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
    gfx->fillRoundRect(35, 35, 152, 95, 10, WHITE);

    if (Wifi_Connection_Flag == true)
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo, 1000))
        {
            Serial.println("Failed to obtain time");
            gfx->setCursor(50, 45);
            gfx->setTextColor(RED);
            gfx->setTextSize(1);
            gfx->print("Time error");
            return;
        }
        Serial.println("Get time success");
        Serial.println(&timeinfo, "%A,%B %d %Y %H:%M:%S"); // Format Output
        gfx->setCursor(50, 45);
        gfx->setTextColor(ORANGE);
        gfx->setTextSize(1);
        gfx->print(&timeinfo, " %Y");
        gfx->setCursor(50, 60);
        gfx->print(&timeinfo, "%B %d");
        gfx->setCursor(50, 75);
        gfx->print(&timeinfo, "%H:%M:%S");
    }
    else
    {
        gfx->setCursor(50, 45);
        gfx->setTextSize(2);
        gfx->setTextColor(RED);
        gfx->print("Network error");
    }

    gfx->setCursor(50, 90);
    gfx->printf("SYS Time:%d", (uint32_t)millis() / 1000);
}
