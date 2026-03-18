/*
 * @Description: SD Card Test Module Implementation for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "sd_test.h"

void SD_Test_Loop(void)
{
    Serial.println("Detecting SD card");

    if (SD.begin(SD_CS, SPI) == false) // SD boots
    {
        gfx->fillRect(0, 50, LCD_WIDTH, 100, WHITE);
        gfx->setTextSize(1);
        gfx->setTextColor(BLACK);
        gfx->setCursor(0, 60);

        Serial.println("SD bus initialization failed !");
        gfx->printf("SD bus init failed!\n");
    }
    else
    {
        gfx->fillRect(0, 50, LCD_WIDTH, 100, WHITE);
        gfx->setTextSize(1);
        gfx->setTextColor(BLACK);
        gfx->setCursor(0, 60);

        Serial.println("SD bus initialization successful !");
        gfx->printf("SD bus init successful!\n");

        uint8_t card_type = SD.cardType();
        uint64_t card_size = SD.cardSize() / (1024 * 1024);
        // uint8_t num_sectors = SD.numSectors();
        switch (card_type)
        {
        case CARD_NONE:
            Serial.println("No SD card attached");
            gfx->printf("No SD card attached\n");
            gfx->printf("SD card test failed\n");

            break;
        case CARD_MMC:
            Serial.print("SD Card Type: ");
            Serial.println("MMC");
            Serial.printf("SD Card Size: %lluMB\n", card_size);
            gfx->printf("SD Card Type:MMC\nSD Card Size:%lluMB\n", card_size);
            gfx->printf("SD card test successful\n");

            break;
        case CARD_SD:
            Serial.print("SD Card Type: ");
            Serial.println("SDSC");
            Serial.printf("SD Card Size: %lluMB\n", card_size);
            gfx->printf("SD Card Type:SDSC\nSD Card Size:%lluMB\n", card_size);
            gfx->printf("SD card tested successful\n");

            break;
        case CARD_SDHC:
            Serial.print("SD Card Type: ");
            Serial.println("SDHC");
            Serial.printf("SD Card Size: %lluMB\n", card_size);
            gfx->printf("SD Card Type:SDHC\nSD Card Size:%lluMB\n", card_size);
            gfx->printf("SD card tested successful\n");

            break;
        default:
            Serial.println("UNKNOWN");
            gfx->printf("UNKNOWN");
            gfx->printf("SD card test failed\n");

            break;
        }
    }
    SD.end();
}

void SD_Test_Display()
{
    gfx->fillScreen(WHITE);

    gfx->setTextSize(2);
    gfx->setTextColor(PURPLE);
    gfx->setCursor(60, 30);
    gfx->printf("SD Info");

    gfx->setCursor(0, 100);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);

    SPI.setFrequency(16000000);
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI);

    GFX_Print_1();
}

bool SD_Test_Loop_Main()
{
    GFX_Print_TEST("SD Test");
    if (Skip_Current_Test == false)
    {
        SD_Test_Display();
        while (1)
        {
            bool temp = false;

            if (millis() > CycleTime)
            {
                SD_Test_Loop();
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
                    GFX_Print_TEST("SD Test");
                    SD_Test_Display();
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
