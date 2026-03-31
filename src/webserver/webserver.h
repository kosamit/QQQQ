/*
 * @Description: Web file manager for SD card
 * @Author: kosamit
 */

#ifndef QQQQ_WEBFILEMANAGER_H
#define QQQQ_WEBFILEMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <FS.h>

extern bool Wifi_Connection_Flag;
extern bool SD_Initialization_Flag;

extern WebServer server;
extern bool webServerRunning;

// Webサーバー初期化・開始
void startWebServer();
void stopWebServer();
void handleWebServer();

#endif // QQQQ_WEBFILEMANAGER_H
