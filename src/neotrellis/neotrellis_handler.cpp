/*
 * @Description: NeoTrellis 4x4 handler for drum pad
 * @Author: kosamit
 */

#include "neotrellis_handler.h"
#include "../midi/midi_handler.h"
#include <Wire.h>

bool neotrellisConnected = false;
Adafruit_NeoTrellis* trellis = nullptr;

// NeoTrellisのキー番号(0-15)をGrid4x4のrow/colに変換
void neotrellisKeyToGrid(uint8_t key, int16_t& row, int16_t& col) {
    row = key / 4;
    col = key % 4;
}

// I2Cバス上のデバイスをスキャン（デバッグ用）
void scanI2C(TwoWire& wire, const char* busName) {
    Serial.printf("I2C scan on %s (SDA=%d, SCL=%d):\n", busName, IIC_SDA, IIC_SCL);
    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        wire.beginTransmission(addr);
        if (wire.endTransmission() == 0) {
            Serial.printf("  Found device at 0x%02X\n", addr);
            count++;
        }
    }
    if (count == 0) {
        Serial.println("  No devices found!");
    }
}

bool initNeoTrellis() {
    // Wire (SDA=5, SCL=6) は Arduino_HWIIC で既に初期化済み
    Serial.println("=== I2C Bus Scan ===");
    scanI2C(Wire, "Wire(5,6)");

    // Wire1 (SDA=43, SCL=44) も試行
    Wire1.begin(43, 44);
    scanI2C(Wire1, "Wire1(43,44)");

    // 両方のバスで NeoTrellis を探す
    TwoWire* buses[] = { &Wire, &Wire1 };
    const char* busNames[] = { "Wire", "Wire1" };
    const uint8_t addrs[] = { 0x2E, 0x23, 0x5A };
    bool found = false;

    for (int b = 0; b < 2 && !found; b++) {
        for (uint8_t i = 0; i < sizeof(addrs) && !found; i++) {
            Serial.printf("Trying NeoTrellis on %s at 0x%02X... ", busNames[b], addrs[i]);
            trellis = new Adafruit_NeoTrellis(addrs[i], buses[b]);
            if (trellis->begin()) {
                Serial.printf("OK!\n");
                found = true;
            } else {
                Serial.printf("fail\n");
                delete trellis;
                trellis = nullptr;
            }
        }
    }

    if (!found) {
        Serial.println("NeoTrellis not found on any bus/address");
        neotrellisConnected = false;
        return false;
    }

    Serial.println("NeoTrellis initialization successfully");
    neotrellisConnected = true;

    // 全16キーのコールバックを登録
    for (uint8_t i = 0; i < 16; i++) {
        trellis->activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
        trellis->activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
        trellis->registerCallback(i, neotrellisCallback);
    }

    // 初期LED状態：薄暗く点灯（待機状態表示）
    for (uint8_t i = 0; i < 16; i++) {
        trellis->pixels.setPixelColor(i, NT_COLOR_IDLE);
    }
    trellis->pixels.show();

    return true;
}

TrellisCallback neotrellisCallback(keyEvent evt) {
    uint8_t key = evt.bit.NUM;
    int16_t row, col;
    neotrellisKeyToGrid(key, row, col);

    if (currentScreen != SCREEN_DRUMPAD || grid == nullptr) {
        return 0;
    }

    if (currentTouchMode == TOUCH_MODE_TOGGLE) {
        // トグルモード：押下時のみトグル
        if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
            grid->toggleCellActive(row, col);
            grid->markCellForRedraw(row, col);
        }
    } else {
        // ホールドモード：押下でアクティブ、リリースで非アクティブ
        if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
            grid->setCellActive(row, col, true);
            grid->markCellForRedraw(row, col);
        } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
            grid->setCellActive(row, col, false);
            grid->markCellForRedraw(row, col);
        }
    }

    // MIDI送信
    sendMidiNotesForChangedCells();

    return 0;
}

void syncNeoTrellisLEDs() {
    if (!neotrellisConnected || trellis == nullptr || grid == nullptr) {
        return;
    }

    for (int16_t row = 0; row < GRID_ROWS; row++) {
        for (int16_t col = 0; col < GRID_COLS; col++) {
            uint8_t key = row * 4 + col;
            if (grid->isCellActive(row, col)) {
                trellis->pixels.setPixelColor(key, NT_COLOR_ACTIVE);
            } else {
                trellis->pixels.setPixelColor(key, NT_COLOR_IDLE);
            }
        }
    }
    trellis->pixels.show();
}

void clearNeoTrellisLEDs() {
    if (!neotrellisConnected || trellis == nullptr) {
        return;
    }

    for (uint8_t i = 0; i < 16; i++) {
        trellis->pixels.setPixelColor(i, NT_COLOR_OFF);
    }
    trellis->pixels.show();
}
