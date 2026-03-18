/*
 * @Description: Music Test Module Implementation for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "music_test.h"

void GFX_Print_Play_Progress(void)
{
    gfx->setTextSize(2);
    gfx->setCursor(35, 270);

    gfx->fillRect(35, 270, 182, 50, WHITE);
    gfx->printf("%d:%d", audio.getAudioCurrentTime() / 60, audio.getAudioCurrentTime() % 60);

    gfx->setCursor(100, 270);
    gfx->printf("->");

    gfx->setCursor(145, 270);
    gfx->printf("%d:%d", audio.getAudioFileDuration() / 60, audio.getAudioFileDuration() % 60);
}

void GFX_Print_Volume()
{
    gfx->setCursor(LCD_WIDTH / 4 + 5, LCD_WIDTH / 4 + 40);
    gfx->setTextSize(3);
    gfx->setTextColor(ORANGE);
    gfx->printf("Volume");
}

void GFX_Print_Music_Volume_Value()
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
    gfx->print(audio.getVolume());
}

void GFX_Print_Play_Failed()
{
    gfx->setCursor(20, 30);
    gfx->setTextSize(1);
    gfx->setTextColor(RED);
    gfx->printf("Play failed\n");
}

void Music_Test_Display()
{
    if (audio.setPinout(MAX98357A_BCLK, MAX98357A_LRCLK, MAX98357A_DATA) != true)
    {
        Serial.println("Music driver initialization failed");
        Music_Start_Playing_Flag = false;
        delay(1000);
    }
    else
    {
        Serial.println("Music driver initialization successfully");

        gfx->fillScreen(WHITE);
        gfx->setCursor(0, 100);
        gfx->setTextSize(1);
        gfx->setTextColor(PURPLE);
        gfx->println("Trying to play music...");

        while (1)
        {
            if (SD.begin(SD_CS, SPI) == true)
            {
                if (SD_File_Download_Check_Flag == true)
                {
                    if (audio.connecttoSD(SD_FILE_NAME_TEMP) == false)
                    {
                        Music_Start_Playing_Flag = false;
                    }
                    else
                    {
                        Music_Start_Playing_Flag = true;
                        break;
                    }
                }
                else
                {
                    if (audio.connecttoSD(SD_FILE_NAME) == false)
                    {
                        Music_Start_Playing_Flag = false;
                    }
                    else
                    {
                        Music_Start_Playing_Flag = true;
                        break;
                    }
                }
            }
            else
            {
                gfx->printf("SD card initialization failed\n");
            }

            gfx->print(".");

            Music_Start_Playing_Count++;
            if (Music_Start_Playing_Count > 10) // 10秒タイムアウト
            {
                Music_Start_Playing_Flag = false;
                break;
            }
            delay(1000);
        }

        Music_Start_Playing_Count = 0;

        while (audio.getAudioCurrentTime() <= 0)
        {
            audio.loop();

            if (millis() > CycleTime)
            {
                Music_Start_Playing_Count++;
                if (Music_Start_Playing_Count > 10) // 10秒後にダウンロードタイムアウト
                {
                    Music_Start_Playing_Flag = false;
                    break;
                }
                CycleTime = millis() + 1000;
            }
        }
    }

    gfx->fillScreen(WHITE);
    GFX_Print_1();

    if (Music_Start_Playing_Flag == true)
    {
        GFX_Print_Volume();
        GFX_Print_Music_Volume_Value();
        GFX_Print_Play_Progress();
    }
    else
    {
        GFX_Print_Play_Failed();
    }
}

bool Music_Test_Loop()
{
    GFX_Print_TEST("SD Music Test");
    if (Skip_Current_Test == false)
    {
        Music_Test_Display();
        while (1)
        {
            bool temp = false;

            if (Music_Start_Playing_Flag == true)
            {
                if (millis() > CycleTime)
                {
                    GFX_Print_Play_Progress();
                    CycleTime = millis() + 1000;
                }

                audio.loop();
            }

            uint8_t fingers_number = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

            if (fingers_number > 0)
            {
                int32_t touch_x = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
                int32_t touch_y = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

                if (touch_x > 20 && touch_x < 100 &&
                    touch_y > (LCD_HEIGHT - (LCD_HEIGHT / 4)) &&
                    touch_y < (LCD_HEIGHT - (LCD_HEIGHT / 4) + 40))
                {
                    Vibration_Start();
                    GFX_Print_TEST("SD Music Test");
                    Music_Test_Display();
                    if (Skip_Current_Test == true)
                    {
                        temp = true;
                    }
                }
                if (touch_x > 120 && touch_x < 200 &&
                    touch_y > (LCD_HEIGHT - (LCD_HEIGHT / 4)) &&
                    touch_y < (LCD_HEIGHT - (LCD_HEIGHT / 4) + 40))
                {
                    Vibration_Start();
                    temp = true;
                }

                if (Music_Start_Playing_Flag == true)
                {
                    if (touch_x > 30 && touch_x < 80 &&
                        touch_y > LCD_HEIGHT / 3 && touch_y < LCD_HEIGHT / 3 + 50)
                    {
                        Volume_Value--;
                        Vibration_Start();
                        if (Volume_Value < 0)
                        {
                            Volume_Value = 0;
                        }
                    }
                    if (touch_x > 140 && touch_x < 190 &&
                        touch_y > LCD_HEIGHT / 3 && touch_y < LCD_HEIGHT / 3 + 50)
                    {
                        Volume_Value++;
                        Vibration_Start();
                        if (Volume_Value > 21)
                        {
                            Volume_Value = 21;
                        }
                    }

                    audio.setVolume(Volume_Value);
                    GFX_Print_Music_Volume_Value();
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
