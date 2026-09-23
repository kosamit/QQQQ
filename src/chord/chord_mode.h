/*
 * @Description: CHORD mode integration layer (screen + input + MIDI).
 *   Wraps the pure music::ChordEngine and connects it to the display,
 *   NeoTrellis and touch input, and BLE-MIDI output.
 * @Author: kosamit
 */

#ifndef CHORD_MODE_H
#define CHORD_MODE_H

#include <Arduino.h>

#include "../music/chord_engine.h"

// CHORD 画面の 4x4 グリッド配置（600x450 ランドスケープ）。
// レイアウト: 左右上下25pxマージン / 左150pxを情報パネル / 4x4=400x400のグリッド。
//   25(左margin) + 150(info) + 400(grid) + 25(右margin) = 600
//   25(上margin) + 400(grid) + 25(下margin) = 450
#define CHORD_CELL_W 100
#define CHORD_CELL_H 100
#define CHORD_MARGIN 25
#define CHORD_INFO_W 150
#define CHORD_GRID_X (CHORD_MARGIN + CHORD_INFO_W)  // 175
#define CHORD_GRID_Y CHORD_MARGIN                   // 25

// 共有される CHORD エンジン（16パッドのコード + 選択 + octave/transpose）。
extern music::ChordEngine chordEngine;

// 画面全体を描画。
void drawChordScreen();

// パッド押下/離し（MIDI 送出 + 演奏状態更新 + 再描画マーク）。pad は 0..15。
void chordPadPress(uint8_t pad);
void chordPadRelease(uint8_t pad);

// タッチ座標からパッドを判定し押下/離しを駆動（単押しモーメンタリ）。
// グリッド外/指なしのリリースも処理する。
void chordHandleTouch(int16_t* xs, int16_t* ys, uint8_t fingerCount);

// 座標 → パッド番号（グリッド外は -1）。
int8_t chordPadAt(int16_t x, int16_t y);

// 情報パネルの上下左右ボタン（D-pad）処理。全指を走査しエッジ検出（1タップ1回）。
// D-pad ボタンに指が乗っていれば true。
//   パッド保持中（グリッド/NeoTrellis）: 上下=root(キー), 左右=tension(コード種類)
//   それ以外                          : 上下=octave, 左右=transpose
bool chordHandleControls(int16_t* xs, int16_t* ys, uint8_t fingerCount);

// dirty なパッド/情報パネルを再描画（displayMutex 保持中に呼ぶこと）。
void chordRedrawDirty();

// NeoTrellis LED を演奏状態に同期。
void chordSyncLEDs();

#endif  // CHORD_MODE_H
