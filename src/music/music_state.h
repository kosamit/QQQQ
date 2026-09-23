/*
 * @Description: Mutable musical parameters (octave / transpose / scale ...)
 *   Ported from picotofu.py global music state.
 *   Pure data + immutable adjust helpers — no Arduino/hardware dependency,
 *   so this compiles and unit-tests on the host too.
 * @Author: kosamit
 */

#ifndef MUSIC_STATE_H
#define MUSIC_STATE_H

#include <stdint.h>

namespace music {

// --- パラメータ範囲（picotofu.py 由来）---
constexpr int8_t OCTAVE_DEFAULT = 5;
constexpr int8_t OCTAVE_MIN = 1;
constexpr int8_t OCTAVE_MAX = 8;

constexpr int8_t DRUMPAD_OCTAVE_DEFAULT = 3;
constexpr int8_t DRUMPAD_OCTAVE_MIN = 0;
constexpr int8_t DRUMPAD_OCTAVE_MAX = 8;

constexpr int8_t TRANSPOSE_DEFAULT = 0;
constexpr int8_t TRANSPOSE_MIN = -11;
constexpr int8_t TRANSPOSE_MAX = 11;

constexpr uint8_t VELOCITY_DEFAULT = 100;
constexpr uint8_t VELOCITY_MIN = 0;
constexpr uint8_t VELOCITY_MAX = 127;

constexpr uint16_t BPM_DEFAULT = 120;

// 現在の音楽状態（可変パラメータの集合）。
// 変更は with*() ヘルパで新しいコピーを返す（不変データ方針）。
struct MusicState {
    int8_t octave = OCTAVE_DEFAULT;
    int8_t drumpadOctave = DRUMPAD_OCTAVE_DEFAULT;
    int8_t transpose = TRANSPOSE_DEFAULT;
    uint8_t velocity = VELOCITY_DEFAULT;
    uint16_t bpm = BPM_DEFAULT;
    uint8_t scaleNum = 0;    // SCALES / SCALE_NAMES のインデックス
    uint8_t diatonicNum = 0; // DiatonicChord のインデックス
};

// direction は通常 +1 / -1。範囲外への変更は無視して同値を返す。
MusicState withOctaveDelta(const MusicState& s, int direction);
MusicState withDrumpadOctaveDelta(const MusicState& s, int direction);
MusicState withTransposeDelta(const MusicState& s, int direction);
// スケール番号は循環（modulo SCALE_COUNT）。
MusicState withScaleDelta(const MusicState& s, int direction);
// ダイアトニック番号は循環（modulo DIATONIC_COUNT）。
MusicState withDiatonicDelta(const MusicState& s, int direction);

}  // namespace music

#endif  // MUSIC_STATE_H
