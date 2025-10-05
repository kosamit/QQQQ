/*
 * @Description: Microphone Test Module Implementation for Qurospad
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "microphone_test.h"

void GFX_Print_Voice_Speaker_Info_Loop(int16_t left_channel, int16_t right_channel)
{
    gfx->fillRect(30, 30, 130, 30, WHITE);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);

    gfx->setCursor(30, 30);
    gfx->printf("Voice Data:");

    gfx->setCursor(30, 40);
    gfx->printf("Left Channel:%d ", left_channel);

    gfx->setCursor(30, 50);
    gfx->printf("Right Channel:%d", right_channel);
}

void Microphone_Test_Display()
{
    gfx->fillScreen(WHITE);

    gfx->setTextSize(2);
    gfx->setTextColor(PURPLE);
    gfx->setCursor(20, 30);
    gfx->printf("Microphone Info");

    pinMode(MSM261_EN, OUTPUT);
    digitalWrite(MSM261_EN, HIGH);

    if (IIS->begin(I2S_MODE_MASTER, AD_IIS_DATA_IN, I2S_CHANNEL_FMT_ONLY_RIGHT,
                   IIS_DATA_BIT, IIS_SAMPLE_RATE) == false)
    {
        Serial.println("MSM261 initialization fail");
        delay(500);
    }
    else
    {
        Serial.println("MSM261 initialization successfully");
    }

    GFX_Print_1();
}

bool Microphone_Test_Loop()
{
    GFX_Print_TEST("Microphone Test");
    if (Skip_Current_Test == false)
    {
        Microphone_Test_Display();
        while (1)
        {
            bool temp = false;

            if (IIS->IIS_Read_Data(IIS_Read_Buff, 100) == true)
            {
                Serial.printf("Right Data:%d", (int16_t)((int16_t)IIS_Read_Buff[2] | (int16_t)IIS_Read_Buff[3] << 8));

                gfx->fillRect(0, 70, 222, 110, WHITE);
                gfx->setTextColor(BLUE);

                gfx->setCursor(20, 100);
                gfx->printf("Right Data:%d", (int16_t)((int16_t)IIS_Read_Buff[2] | (int16_t)IIS_Read_Buff[3] << 8));
                delay(100);
            }
            else
            {
                gfx->setTextColor(RED);
                gfx->fillRect(0, 70, 222, 110, WHITE);

                gfx->setCursor(20, 100);
                gfx->printf("Microphone Data: fail");
                delay(100);
            }

            uint8_t fingers_number = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

            if (fingers_number > 0)
            {
                int32_t touch_x = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
                int32_t touch_y = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

                if (touch_x > 20 && touch_x < 100 &&
                    touch_y > (LCD_HEIGHT - (LCD_HEIGHT / 4)) && touch_y < (LCD_HEIGHT - (LCD_HEIGHT / 4) + 40))
                {
                    Vibration_Start();
                    GFX_Print_TEST("Microphone Test");
                    Microphone_Test_Display();
                    if (Skip_Current_Test == true)
                    {
                        temp = true;
                    }
                }
                if (touch_x > 120 && touch_x < 200 &&
                    touch_y > (LCD_HEIGHT - (LCD_HEIGHT / 4)) && touch_y < (LCD_HEIGHT - (LCD_HEIGHT / 4) + 40))
                {
                    Vibration_Start();
                    temp = true;
                }
            }

            if (temp == true)
            {
                IIS->end();
                break;
            }
        }
    }
    return true;
}
