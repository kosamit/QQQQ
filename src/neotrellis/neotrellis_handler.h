/*
 * @Description: NeoTrellis 4x4 handler for drum pad
 * @Author: kosamit
 */

#ifndef NEOTRELLIS_HANDLER_H
#define NEOTRELLIS_HANDLER_H

#include "../globals.h"
#include <Adafruit_NeoTrellis.h>

// NeoTrellis I2Cアドレス（デフォルト: 0x2E）
#define NEOTRELLIS_I2C_ADDR 0x2E

// NeoTrellis LED色定義（NeoPixel GRBフォーマット）
#define NT_COLOR_OFF     0x000000
#define NT_COLOR_ACTIVE  0x00FF00   // 緑（アクティブ）
#define NT_COLOR_PRESSED 0xFF8000   // オレンジ（押下中）
#define NT_COLOR_IDLE    0x050505   // 薄暗い白（待機中）

// NeoTrellisの状態
extern bool neotrellisConnected;
extern Adafruit_NeoTrellis* trellis;

// 初期化
bool initNeoTrellis();

// ボタンイベントのコールバック
TrellisCallback neotrellisCallback(keyEvent evt);

// LED同期（Grid4x4の状態をNeoTrellis LEDに反映）
void syncNeoTrellisLEDs();

// 全LED消灯
void clearNeoTrellisLEDs();

// NeoTrellisのrow/colからGrid4x4のrow/colへ変換
void neotrellisKeyToGrid(uint8_t key, int16_t& row, int16_t& col);

#endif // NEOTRELLIS_HANDLER_H
