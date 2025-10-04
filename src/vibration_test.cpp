/*
 * @Description: Vibration Motor Test Module Implementation for Qurospad
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "vibration_test.h"

void GFX_Print_Strength()
{
    gfx->setCursor(LCD_WIDTH / 4 - 15, LCD_WIDTH / 4 + 40);
    gfx->setTextSize(3);
    gfx->setTextColor(ORANGE);
    gfx->printf("Strength");
}

void GFX_Print_Motor_Strength_Value()
{
    gfx->fillRect(30, LCD_HEIGHT / 3, 50, 50, PALERED);
    gfx->drawRect(30, LCD_HEIGHT / 3, 50, 50, PURPLE);
    gfx->fillRect(140, LCD_HEIGHT / 3, 50, 50, PALERED);
    gfx->drawRect(140, LCD_HEIGHT / 3, 50, 50, PURPLE);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(50, (LCD_HEIGHT / 3) + 17);
    gfx->printf("-");
    gfx->setCursor(160, (LCD_HEIGHT / 3) + 17);
    gfx->printf("+");

    gfx->fillRect(LCD_WIDTH / 2 - 13, LCD_HEIGHT / 3 + 13, 35, 35, WHITE);
    gfx->setCursor(LCD_WIDTH / 2 - 13, LCD_HEIGHT / 3 + 13);
    gfx->setTextSize(3);
    gfx->setTextColor(RED);
    gfx->print(Strength_Value);
}

void Vibration_Test_Display()
{
    gfx->fillScreen(WHITE);

    gfx->setTextSize(2);
    gfx->setTextColor(PURPLE);
    gfx->setCursor(25, 30);
    gfx->printf("Vibration Info");

    GFX_Print_Strength();
    GFX_Print_Motor_Strength_Value();

    GFX_Print_1();
}

bool Vibration_Test_Loop()
{
    GFX_Print_TEST("Vibration Motor Test");
    if (Skip_Current_Test == false)
    {
        Vibration_Test_Display();
        while (1)
        {
            bool temp = false;

            uint8_t fingers_number = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

            if (fingers_number > 0)
            {
                int32_t touch_x = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
                int32_t touch_y = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

                if (touch_x > 20 && touch_x < 100 &&
                    touch_y > (LCD_HEIGHT - (LCD_HEIGHT / 4)) && touch_y < (LCD_HEIGHT - (LCD_HEIGHT / 4) + 40))
                {
                    Vibration_Start();
                    GFX_Print_TEST("Vibration Motor Test");
                    Vibration_Test_Display();
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

                if (touch_x > 30 && touch_x < 80 &&
                    touch_y > LCD_HEIGHT / 3 && touch_y < LCD_HEIGHT / 3 + 50)
                {
                    Strength_Value--;
                    if (Strength_Value < 0)
                    {
                        Strength_Value = 0;
                    }
                }
                if (touch_x > 140 && touch_x < 190 &&
                    touch_y > LCD_HEIGHT / 3 && touch_y < LCD_HEIGHT / 3 + 50)
                {
                    Strength_Value++;
                    if (Strength_Value > 20)
                    {
                        Strength_Value = 20;
                    }
                }

                ledcWrite(2, Strength_Value * 10);
                GFX_Print_Motor_Strength_Value();
                delay(200);
            }

            if (temp == true)
            {
                ledcWrite(2, 0);
                break;
            }
        }
    }
    return true;
}
