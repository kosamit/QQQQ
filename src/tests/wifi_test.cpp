/*
 * @Description: WiFi Test Module Implementation for Qurospad
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "wifi_test.h"

void Wifi_STA_Test(void)
{
    String text;
    int wifi_num = 0;

    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 0);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);

    Serial.println("\nScanning wifi");
    gfx->printf("\n\n\nScanning wifi\n");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    delay(100);

    wifi_num = WiFi.scanNetworks();
    if (wifi_num == 0)
    {
        text = "\nWiFi scan complete !\nNo wifi discovered.\n";
    }
    else
    {
        text = "\nWiFi scan complete !\n";
        text += wifi_num;
        text += " wifi discovered.\n\n";

        for (int i = 0; i < wifi_num; i++)
        {
            text += (i + 1);
            text += ": ";
            text += WiFi.SSID(i);
            text += " (";
            text += WiFi.RSSI(i);
            text += ")";
            text += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " \n" : "*\n";
            delay(10);
        }
    }

    Serial.println(text);
    gfx->println(text);

    delay(3000);
    text.clear();
    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 50);

    Serial.print("Connecting to ");
    gfx->printf("Connecting to\n");

    Serial.print(WIFI_SSID);
    gfx->printf("%s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t last_tick = millis();

    Wifi_Connection_Flag = true;

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        gfx->printf(".");
        delay(100);

        if (millis() - last_tick > WIFI_CONNECT_WAIT_MAX)
        {
            Wifi_Connection_Flag = false;
            break;
        }
    }

    if (Wifi_Connection_Flag == true)
    {
        Serial.print("\nThe connection was successful ! \nTakes ");
        gfx->printf("\nThe connection was successful ! \nTakes ");
        Serial.print(millis() - last_tick);
        gfx->print(millis() - last_tick);
        Serial.println(" ms\n");
        gfx->printf(" ms\n");

        gfx->setTextColor(PURPLE);
        gfx->printf("\nWifi test passed!");
    }
    else
    {
        gfx->setTextColor(RED);
        gfx->printf("\nWifi test error!\n");
    }
}

void WIFI_HTTP_Download_SD_File(void)
{
    // 初始化HTTP客户端
    HTTPClient http;
    http.begin(fileDownloadUrl);
    // 获取重定向的URL
    const char *headerKeys[] = {"Location"};
    http.collectHeaders(headerKeys, 1);

    // 记录下载开始时间
    size_t startTime = millis();
    // 无用时间
    size_t uselessTime = 0;

    // 发起GET请求
    int httpCode = http.GET();

    while (httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND)
    {
        String newUrl = http.header("Location");
        Serial.printf("Redirecting to: %s\n", newUrl.c_str());
        http.end(); // 关闭旧的HTTP连接

        // 使用新的URL重新发起GET请求
        http.begin(newUrl);
        httpCode = http.GET();
    }

    if (httpCode == HTTP_CODE_OK)
    {
        // 获取文件大小
        size_t fileSize = http.getSize();
        Serial.printf("Starting file download...\n");
        Serial.printf("file size: %f MB\n", fileSize / 1024.0 / 1024.0);
        gfx->printf("\n\n\nStarting file download...\n");
        gfx->printf("file size: %f MB\n", fileSize / 1024.0 / 1024.0);

        SD.remove(SD_FILE_NAME_TEMP);

        File file;
        // 打开文件，如果没有文件就创建文件
        file = SD.open(SD_FILE_NAME_TEMP, FILE_WRITE);

        if (file != 0)
        {
            // 读取HTTP响应
            WiFiClient *stream = http.getStreamPtr();

            size_t temp_count_s = 0;
            size_t temp_fileSize = fileSize;
            // uint8_t *buf_1 = (uint8_t *)heap_caps_malloc(64 * 1024, MALLOC_CAP_SPIRAM);
            uint8_t buf_1[4096] = {0};
            CycleTime = millis() + 3000; // 开始计时
            while (http.connected() && (temp_fileSize > 0 || temp_fileSize == -1))
            {
                // 获取可用数据的大小
                size_t availableSize = stream->available();
                if (availableSize)
                {
                    // temp_fileSize -= stream->read(buf_1, min(availableSize, (size_t)(64 * 1024)));
                    temp_fileSize -= stream->read(buf_1, min(availableSize, (size_t)(4096)));

                    // file.write(buf_1, min(availableSize, (size_t)(64 * 1024)));
                    file.write(buf_1, min(availableSize, (size_t)(4096)));

                    if (millis() > CycleTime)
                    {
                        size_t temp_time_1 = millis();
                        temp_count_s += 3;
                        Serial.printf("Download speed: %f KB/s\n", ((fileSize - temp_fileSize) / 1024.0) / temp_count_s);
                        Serial.printf("Remaining file size: %f MB\n\n", temp_fileSize / 1024.0 / 1024.0);

                        gfx->fillRect(0, 60, 200, 68, WHITE);
                        gfx->setCursor(0, 60);
                        gfx->printf("Speed: %f KB/s\n", ((fileSize - temp_fileSize) / 1024.0) / temp_count_s);
                        gfx->printf("Size: %f MB\n\n", temp_fileSize / 1024.0 / 1024.0);

                        delay(1);

                        if (temp_count_s > 50)
                        {
                            break;
                        }

                        CycleTime = millis() + 3000;
                        size_t temp_time_2 = millis();

                        uselessTime = uselessTime + (temp_time_2 - temp_time_1);
                    }
                }
                // delay(1);
            }

            // 关闭HTTP客户端
            http.end();

            // 记录下载结束时间并计算总花费时间
            size_t endTime = millis();

            file.close();
            file = SD.open(SD_FILE_NAME_TEMP);
            size_t temp_file_wifi_download_size = 0;
            if (file != 0)
            {
                temp_file_wifi_download_size = file.size();
            }
            file.close();
            SD.end();
            Serial.println("Writing SD card data completed");

            Serial.printf("Download completed!\n");
            Serial.printf("Total download time: %f s\n", (endTime - startTime - uselessTime) / 1000.0);
            Serial.printf("Average download speed: %f KB/s\n", (fileSize / 1024.0) / ((endTime - startTime - uselessTime) / 1000.0));

            gfx->printf("Completed!\n");
            gfx->printf("Time: %f s\n", (endTime - startTime - uselessTime) / 1000.0);
            gfx->printf("Speed: %f KB/s\n", (fileSize / 1024.0) / ((endTime - startTime - uselessTime) / 1000.0));

            if (temp_file_wifi_download_size == SD_FILE_SIZE)
            {
                SD_File_Download_Check_Flag = true;

                Serial.printf("\nFile verification successful\n");
                gfx->printf("\nFile verification successful\n");
            }
            else
            {
                SD_File_Download_Check_Flag = false;
                Serial.printf("\nFile verification failed\n");
                gfx->printf("\nFile verification failed\n");
            }

            Serial.printf("Original file size: %d Bytes\n", SD_FILE_SIZE);
            Serial.printf("Wifi download size: %d Bytes\n", temp_file_wifi_download_size);
            gfx->printf("Original file size: %d Bytes\n", SD_FILE_SIZE);
            gfx->printf("Wifi download size: %d Bytes\n", temp_file_wifi_download_size);
        }
        else
        {
            Serial.printf("Fail to open file\n");
            gfx->printf("Fail to open file\n\n");
        }
    }
    else
    {
        Serial.printf("Failed to download\n");
        Serial.printf("Error httpCode: %d \n", httpCode);

        gfx->printf("Failed to download\n");
        gfx->printf("Error httpCode: %d \n\n", httpCode);
    }
}

void PrintLocalTime(void)
{
    struct tm timeinfo;
    uint32_t temp_buf = 0;
    bool temp_buf_2 = true;

    while (getLocalTime(&timeinfo) == false)
    {
        Serial.printf("\n.");
        gfx->printf("\n.");
        configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
        temp_buf++;
        if (temp_buf > 5)
        {
            temp_buf_2 = false;
            break;
        }
    }
    if (temp_buf_2 == false)
    {
        Serial.println("Failed to obtain time");
        gfx->setCursor(80, 200);
        gfx->setTextColor(RED);
        gfx->print("Failed to obtain time!");
        return;
    }

    Serial.println("Get time success");
    Serial.println(&timeinfo, "%A,%B %d %Y %H:%M:%S"); // Format Output
    gfx->setCursor(80, 200);
    gfx->setTextColor(PURPLE);
    gfx->print(&timeinfo, " %Y");
    gfx->setCursor(80, 215);
    gfx->print(&timeinfo, "%B %d");
    gfx->setCursor(80, 230);
    gfx->print(&timeinfo, "%H:%M:%S");
}

void WiFi_Test_Display()
{
    // This function is handled by Wifi_STA_Test() which displays the WiFi test interface
}

bool WiFi_Test_Loop()
{
    GFX_Print_TEST("WIFI STA Test");
    if (Skip_Current_Test == false)
    {
        Wifi_STA_Test();

        if (Wifi_Connection_Flag == true)
        {
            // Obtain and set the time from the network time server
            // After successful acquisition, the chip will use the RTC clock to update the holding time
            configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
            PrintLocalTime();

            delay(2000);

            gfx->fillScreen(WHITE);
            gfx->setTextColor(BLACK);
            gfx->setCursor(0, 0);

            SD.end();
            SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
            if (SD.begin(SD_CS, SPI) == true)
            {
                SD_Initialization_Flag = true;
                WIFI_HTTP_Download_SD_File();
            }
            else
            {
                SD_Initialization_Flag = false;
                Serial.printf("SD card initialization failed\n");
                gfx->print("\n\n\nSD card initialization failed");
            }
        }
        else
        {
            gfx->setCursor(80, 200);
            gfx->setTextColor(RED);
            gfx->print("Not connected to the network");
        }

        GFX_Print_1();
    }

    while (1)
    {
        bool temp = false;

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
                GFX_Print_TEST("WIFI STA Test");
                Wifi_STA_Test();
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
        }

        if (temp == true)
        {
            break;
        }
    }
    return true;
}
