/*
 * @Description: Global state definitions
 * @Author: kosamit
 */

#include "globals.h"

// グリッドオブジェクト
Grid4x4* grid = nullptr;

// 前回のセル状態を保持（MIDI送信用）
bool prevCellState[GRID_ROWS * GRID_COLS] = {false};

// Bluetooth接続状態
bool bleConnected = false;
bool bleAdvertising = false;
unsigned long lastBleStatusUpdate = 0;

// 画面モード
ScreenMode currentScreen = SCREEN_MENU;

// メニュー項目
MenuItem menuItems[] = {
    {"Drum Pad", SCREEN_DRUMPAD, 90, 10, 300, 40},
    {"Music", SCREEN_MUSIC, 90, 55, 300, 40},
    {"Bluetooth", SCREEN_BLUETOOTH, 90, 100, 300, 40},
    {"About", SCREEN_ABOUT, 90, 145, 300, 40}
};

// モード切り替えボタン
ModeButton modeButton = {
    10, 10, 120, 40,
    "Mode: TOGGLE",
    "Mode: HOLD"
};

// Bluetoothトグルボタン
BluetoothButton bleButton = {
    350, 172, 120, 40
};

// 現在のタッチモード
TouchMode currentTouchMode = TOUCH_MODE_TOGGLE;

// 音楽プレイヤー用
char musicFiles[MUSIC_MAX_FILES][64] = {};
int16_t musicFileCount = 0;
int16_t musicScrollOffset = 0;
int16_t musicSelectedIndex = -1;
bool musicIsPlaying = false;

// FreeRTOS オブジェクト
TaskHandle_t touchTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t clockTaskHandle = NULL;
TaskHandle_t neotrellisTaskHandle = NULL;
QueueHandle_t touchEventQueue = NULL;
SemaphoreHandle_t displayMutex = NULL;
