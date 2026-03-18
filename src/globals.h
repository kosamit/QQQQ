/*
 * @Description: Global state shared across modules
 * @Author: kosamit
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "common/common_config.h"
#include "lib/grid.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// 画面モード定義
enum ScreenMode {
    SCREEN_MENU,           // メニュー画面
    SCREEN_DRUMPAD,        // ドラムパッド画面
    SCREEN_BLUETOOTH,      // Bluetooth設定画面
    SCREEN_ABOUT           // About画面
};

// メニュー項目定義
struct MenuItem {
    const char* label;
    ScreenMode targetScreen;
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
};

// モード切り替えボタンの定義
struct ModeButton {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    const char* label_toggle;
    const char* label_hold;
};

// Bluetoothトグルボタンの定義
struct BluetoothButton {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
};

// タッチイベント構造体（マルチタッチ対応）
struct TouchEvent {
    uint8_t finger_count;      // タッチしている指の数
    int16_t x[5];              // 各タッチポイントのX座標
    int16_t y[5];              // 各タッチポイントのY座標
};

// BLE-MIDI インスタンス（BLEMIDI_CREATE_INSTANCE マクロで main.ino に定義）
typedef bleMidi::BLEMIDI_Transport<bleMidi::BLEMIDI_ESP32_NimBLE<bleMidi::DefaultSettings>, bleMidi::DefaultSettings> BleMidiTransport;
typedef midi::MidiInterface<BleMidiTransport, bleMidi::MySettings> BleMidiInterface;

extern BleMidiTransport BLEMIDI;
extern BleMidiInterface MIDI;

// グローバル変数 extern 宣言
extern Grid4x4* grid;
extern bool prevCellState[];
extern bool bleConnected;
extern bool bleAdvertising;
extern unsigned long lastBleStatusUpdate;
extern ScreenMode currentScreen;
extern MenuItem menuItems[];
extern ModeButton modeButton;
extern BluetoothButton bleButton;
extern TouchMode currentTouchMode;

// FreeRTOS オブジェクト
extern TaskHandle_t touchTaskHandle;
extern TaskHandle_t displayTaskHandle;
extern TaskHandle_t clockTaskHandle;
extern QueueHandle_t touchEventQueue;
extern SemaphoreHandle_t displayMutex;

#endif // GLOBALS_H
