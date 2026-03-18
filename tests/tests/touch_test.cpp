/*
 * @Description: Touch Test Module Implementation for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "touch_test.h"

void GFX_Print_Touch_Info_Loop()
{
    int32_t touch_x_1 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH1_COORDINATE_X);
    int32_t touch_y_1 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH1_COORDINATE_Y);
    int32_t touch_x_2 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH2_COORDINATE_X);
    int32_t touch_y_2 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH2_COORDINATE_Y);
    int32_t touch_x_3 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH3_COORDINATE_X);
    int32_t touch_y_3 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH3_COORDINATE_Y);
    int32_t touch_x_4 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH4_COORDINATE_X);
    int32_t touch_y_4 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH4_COORDINATE_Y);
    int32_t touch_x_5 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH5_COORDINATE_X);
    int32_t touch_y_5 = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH5_COORDINATE_Y);
    uint8_t fingers_number = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

    gfx->fillRect(0, 40, LCD_WIDTH, LCD_HEIGHT / 2, WHITE);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);

    gfx->setCursor(20, 50);
    gfx->printf("ID: %#X ", (int32_t)CST226SE->IIC_Device_ID());

    gfx->setCursor(20, 65);
    gfx->printf("Fingers Number:%d ", fingers_number);

    gfx->setCursor(20, 80);
    gfx->printf("Touch X1:%d Y1:%d ", touch_x_1, touch_y_1);
    gfx->setCursor(20, 95);
    gfx->printf("Touch X2:%d Y2:%d ", touch_x_2, touch_y_2);
    gfx->setCursor(20, 110);
    gfx->printf("Touch X3:%d Y3:%d ", touch_x_3, touch_y_3);
    gfx->setCursor(20, 125);
    gfx->printf("Touch X4:%d Y4:%d ", touch_x_4, touch_y_4);
    gfx->setCursor(20, 140);
    gfx->printf("Touch X5:%d Y5:%d ", touch_x_5, touch_y_5);
}

void Touch_Test_Display()
{
    gfx->fillScreen(WHITE);

    gfx->setTextSize(2);
    gfx->setTextColor(PURPLE);
    gfx->setCursor(50, 20);
    gfx->printf("Touch Info");

    GFX_Print_Touch_Info_Loop();
    GFX_Print_1();
}

bool Touch_Test_Loop()
{
    GFX_Print_TEST("Touch Test");
    if (Skip_Current_Test == false)
    {
        Touch_Test_Display();

        while (1)
        {
            bool temp = false;

            uint8_t fingers_number = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

            if (fingers_number > 0)
            {
                int32_t touch_x = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
                int32_t touch_y = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

                GFX_Print_Touch_Info_Loop();
                if (touch_x > 20 && touch_x < 100 &&
                    touch_y > (LCD_HEIGHT - (LCD_HEIGHT / 4)) && touch_y < (LCD_HEIGHT - (LCD_HEIGHT / 4) + 40))
                {
                    Vibration_Start();
                    GFX_Print_TEST("Touch Test");
                    Touch_Test_Display();
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
                break;
            }
        }
    }
    return true;
}
