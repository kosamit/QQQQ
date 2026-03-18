/*
 * @Description: WiFi Test Module for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#ifndef WIFI_TEST_H
#define WIFI_TEST_H

#include "../common/common_config.h"

// WiFi Test Functions
void Wifi_STA_Test(void);
void WIFI_HTTP_Download_SD_File(void);
void PrintLocalTime(void);
void WiFi_Test_Display();
bool WiFi_Test_Loop();

#endif // WIFI_TEST_H
