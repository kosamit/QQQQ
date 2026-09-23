/*
 * @Description: NeoTrellis 4x4 handler for drum pad
 * @Author: kosamit
 */

#include "neotrellis_handler.h"
#include "../midi/midi_handler.h"
#include "../chord/chord_mode.h"
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

// 指定バスで NeoTrellis(seesaw) を探す。アドレス候補はジャンパで 0x2E..0x31。
// 0x5A(touch)/0x6A(PMU) は別デバイスなので探索しない。見つかれば trellis に保持。
static bool probeNeoTrellis(TwoWire& bus, const char* busName) {
    const uint8_t addrs[] = { 0x2E, 0x2F, 0x30, 0x31 };
    for (uint8_t i = 0; i < sizeof(addrs); i++) {
        Serial.printf("Trying NeoTrellis on %s at 0x%02X... ", busName, addrs[i]);
        trellis = new Adafruit_NeoTrellis(addrs[i], &bus);
        if (trellis->begin()) {
            Serial.printf("OK!\n");
            return true;
        }
        Serial.printf("fail\n");
        delete trellis;
        trellis = nullptr;
    }
    return false;
}

// 副バス(GPIO43/44)を触る前に、両線がプルアップされているか(=機器が接続済み)を
// 確認する。何も繋がっていないとフローティングで I2C スキャンがハングするため、
// プルアップが無ければスキャンしない安全弁。
static bool busHasPullups(uint8_t sda, uint8_t scl) {
    pinMode(sda, INPUT);
    pinMode(scl, INPUT);
    delayMicroseconds(50);
    return digitalRead(sda) == HIGH && digitalRead(scl) == HIGH;
}

bool initNeoTrellis() {
    // 一次バス: Wire = GPIO6/7 (P4/I2C コネクタ, touch/PMU と共有)。
    Serial.println("=== I2C Bus Scan (Wire, GPIO6/7) ===");
    scanI2C(Wire, "Wire(6,7)");
    bool found = probeNeoTrellis(Wire, "Wire(6/7)");

    // 二次バス: Wire1 = GPIO43/44 (P2/UART コネクタを独立 I2C として使用)。
    // touch/PMU バスと分離できる利点あり。プルアップがある時だけ触る(ハング防止)。
    if (!found) {
        if (busHasPullups(43, 44)) {
            Wire1.begin(43, 44);
            Wire1.setTimeOut(50);  // フローティング時のスキャン待ちを短時間で打ち切り
            found = probeNeoTrellis(Wire1, "Wire1(43/44)");
        } else {
            Serial.println("Wire1(43/44): no pull-ups (nothing connected), skip");
        }
    }

    if (!found) {
        Serial.println("NeoTrellis not found on Wire(6/7) or Wire1(43/44)");
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

    // コード演奏画面：パッド番号=コードスロット番号として発音
    if (currentScreen == SCREEN_CHORD) {
        if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
            chordPadPress(key);
        } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
            chordPadRelease(key);
        }
        return 0;
    }

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
