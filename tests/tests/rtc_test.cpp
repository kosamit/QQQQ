/*
 * @Description: RTC Test Module Implementation for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "rtc_test.h"

void GFX_Print_RTC_Info_Loop()
{
    gfx->fillRect(0, 50, LCD_WIDTH, LCD_HEIGHT / 2, WHITE);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);

    gfx->setCursor(20, 65);
    gfx->printf("PCF85063  Weekday: %s\n",
                PCF85063->IIC_Read_Device_State(PCF85063->Arduino_IIC_RTC::Status_Information::RTC_WEEKDAYS_DATA).c_str());
    gfx->setCursor(20, 80);
    gfx->printf("PCF85063  Year: %d\n",
                (int32_t)PCF85063->IIC_Read_Device_Value(PCF85063->Arduino_IIC_RTC::Value_Information::RTC_YEARS_DATA));
    gfx->setCursor(20, 95);
    gfx->printf("PCF85063 Date: %d.%d\n",
                (int32_t)PCF85063->IIC_Read_Device_Value(PCF85063->Arduino_IIC_RTC::Value_Information::RTC_MONTHS_DATA),
                (int32_t)PCF85063->IIC_Read_Device_Value(PCF85063->Arduino_IIC_RTC::Value_Information::RTC_DAYS_DATA));
    gfx->setCursor(20, 110);
    gfx->printf("PCF85063 Time: %d:%d:%d\n",
                (int32_t)PCF85063->IIC_Read_Device_Value(PCF85063->Arduino_IIC_RTC::Value_Information::RTC_HOURS_DATA),
                (int32_t)PCF85063->IIC_Read_Device_Value(PCF85063->Arduino_IIC_RTC::Value_Information::RTC_MINUTES_DATA),
                (int32_t)PCF85063->IIC_Read_Device_Value(PCF85063->Arduino_IIC_RTC::Value_Information::RTC_SECONDS_DATA));
}

void GFX_Print_RTC_Switch_Info()
{
    gfx->fillRect(LCD_WIDTH / 4, LCD_HEIGHT - (LCD_HEIGHT / 3) - 30, LCD_WIDTH / 2, 50, PALERED);
    gfx->drawRect(LCD_WIDTH / 4, LCD_HEIGHT - (LCD_HEIGHT / 3) - 30, LCD_WIDTH / 2, 50, RED);
    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);

    gfx->setCursor(LCD_WIDTH / 4 + 30, LCD_HEIGHT - (LCD_HEIGHT / 3) - 40 + 30);
    gfx->printf("RTC Reset");
}

void RTC_Test_Display()
{
    gfx->fillScreen(WHITE);

    gfx->setTextSize(2);
    gfx->setTextColor(PURPLE);
    gfx->setCursor(60, 30);
    gfx->printf("RTC Info");

    GFX_Print_RTC_Switch_Info();
    GFX_Print_1();
}

bool RTC_Test_Loop()
{
    GFX_Print_TEST("RTC Test");
    if (Skip_Current_Test == false)
    {
        RTC_Test_Display();
        while (1)
        {
            bool temp = false;

            if (millis() > CycleTime)
            {
                GFX_Print_RTC_Info_Loop();
                CycleTime = millis() + 1000;
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
                    GFX_Print_TEST("RTC Test");
                    RTC_Test_Display();
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

                if (touch_x > (LCD_WIDTH / 4) && touch_x < (LCD_WIDTH / 4 + LCD_WIDTH / 2) &&
                    touch_y > (LCD_HEIGHT - (LCD_HEIGHT / 3) - 30) && touch_y < (LCD_HEIGHT - (LCD_HEIGHT / 3) - 30 + 50))
                {
                    // 時間をリセット
                    //  RTCを無効化
                    PCF85063->IIC_Write_Device_State(PCF85063->Arduino_IIC_RTC::Device::RTC_CLOCK_RTC,
                                                     PCF85063->Arduino_IIC_RTC::Device_State::RTC_DEVICE_OFF);
                    // クロックセンサーで秒を設定
                    PCF85063->IIC_Write_Device_Value(PCF85063->Arduino_IIC_RTC::Device_Value::RTC_SET_SECOND_DATA,
                                                     58);
                    // クロックセンサーで分を設定
                    PCF85063->IIC_Write_Device_Value(PCF85063->Arduino_IIC_RTC::Device_Value::RTC_SET_MINUTE_DATA,
                                                     59);
                    // クロックセンサーで時を設定
                    PCF85063->IIC_Write_Device_Value(PCF85063->Arduino_IIC_RTC::Device_Value::RTC_SET_HOUR_DATA,
                                                     23);
                    // クロックセンサーで日を設定
                    PCF85063->IIC_Write_Device_Value(PCF85063->Arduino_IIC_RTC::Device_Value::RTC_SET_DAY_DATA,
                                                     31);
                    // クロックセンサーで月を設定
                    PCF85063->IIC_Write_Device_Value(PCF85063->Arduino_IIC_RTC::Device_Value::RTC_SET_MONTH_DATA,
                                                     12);
                    // クロックセンサーで年を設定
                    PCF85063->IIC_Write_Device_Value(PCF85063->Arduino_IIC_RTC::Device_Value::RTC_SET_YEAR_DATA,
                                                     99);
                    // RTCを有効化
                    PCF85063->IIC_Write_Device_State(PCF85063->Arduino_IIC_RTC::Device::RTC_CLOCK_RTC,
                                                     PCF85063->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

                    Vibration_Start();
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
