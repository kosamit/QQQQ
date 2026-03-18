/*
 * @Description: Common configuration and definitions for QQQQ tests
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "Wire.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "Arduino_DriveBus_Library.h"
#include "Audio.h"
#include <SPI.h>
#include <SD.h>
#include <FS.h>

// Software Information
#define SOFTWARE_NAME "Original_Test"
#define SOFTWARE_LASTEDITTIME "202412261832"
#define BOARD_VERSION "V1.0"

// Audio Configuration
#define IIS_SAMPLE_RATE 44100 // 44.1 KHz
#define IIS_DATA_BIT 16       // 16-bit data

// WiFi Configuration
#define WIFI_SSID "xinyuandianzi"
#define WIFI_PASSWORD "AA15994823428"
#define WIFI_CONNECT_WAIT_MAX (10000)

// NTP Configuration
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define NTP_SERVER3 "asia.pool.ntp.org"
#define GMT_OFFSET_SEC 8 * 3600 // UTC+8
#define DAY_LIGHT_OFFSET_SEC 0

// SD Card Configuration
#define SD_FILE_NAME "/music.mp3"
#define SD_FILE_NAME_TEMP "/music_temp.mp3"
#define SD_FILE_SIZE 3638546

// File Download URL
extern const char *fileDownloadUrl;

// Global Variables
extern bool Wifi_Connection_Flag;
extern bool SD_Initialization_Flag;
extern bool SD_File_Download_Check_Flag;
extern char IIS_Read_Buff[100];
extern size_t CycleTime;
extern uint8_t Image_Flag;
extern int8_t Volume_Value;
extern uint8_t Music_Start_Playing_Count;
extern bool Music_Start_Playing_Flag;
extern int8_t Strength_Value;
extern bool Skip_Current_Test;

// Hardware Objects
extern Audio audio;
extern Arduino_DataBus *bus;
extern Arduino_GFX *gfx;
extern std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus;
extern std::shared_ptr<Arduino_IIS_DriveBus> IIS_Bus;
extern std::unique_ptr<Arduino_IIS> IIS;
extern std::unique_ptr<Arduino_IIC> CST226SE;
extern std::unique_ptr<Arduino_IIC> SY6970;
extern std::unique_ptr<Arduino_IIC> PCF85063;

// Common Functions
void Vibration_Start(void);
void Skip_Test_Loop();
void GFX_Print_TEST(String s);
void GFX_Print_1();
void GFX_Print_2();
void GFX_Print_FINISH();
void GFX_Print_START();
void GFX_Print_Time_Info_Loop();

// Interrupt Handlers
void Arduino_IIC_Touch_Interrupt(void);
void Arduino_IIC_RTC_Interrupt(void);

#endif // COMMON_CONFIG_H
