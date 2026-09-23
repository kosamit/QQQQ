/*
 * @Description: Music theory data + note-calculation foundation.
 *   Ported from picotofu (tension.py, diatonic_chord.py, and the
 *   note/scale/drumpad helpers in picotofu.py).
 *   Pure C++ (no Arduino/hardware) so it unit-tests on the host.
 * @Author: kosamit
 */

#ifndef MUSIC_THEORY_H
#define MUSIC_THEORY_H

#include <stdint.h>
#include "music_state.h"

namespace music {

// ---- ノート ----
constexpr uint8_t NOTE_SIZE = 12;
// "C","C#",...,"B"。index は 0..11。
const char* noteName(uint8_t semitone);

// ---- スケール ----
constexpr uint8_t SCALE_COUNT = 3;  // Major / Minor / Dim
const char* scaleName(uint8_t index);
// スケール構成音（ルートからの半音差、末尾にオクターブ上を含む）。
// 鍵盤ハイライト用。count に構成音数を返す。
const int8_t* scaleTones(uint8_t index, uint8_t& count);

// ---- テンション（コードタイプ）----
constexpr uint8_t TENSION_COUNT = 18;
// name はコード表記（"maj","7","min",... 末尾 index17 は単音で ""）。
const char* tensionName(uint8_t index);
// fingers（ルートからの半音差配列）を返し、count に構成音数を設定。
const int8_t* tensionFingers(uint8_t index, uint8_t& count);

// ---- ダイアトニックコード ----
constexpr uint8_t DIATONIC_COUNT = 4;  // Major / Major7th / Minor / Minor7th
const char* diatonicName(uint8_t index);
// pattern（各度数に対応する tension インデックスの配列, 21要素=7度×3oct）。
const uint8_t* diatonicPattern(uint8_t index);

// ---- プリセットコード（CHORD モードの16パッド初期値）----
// picotofu sd/chords.csv 由来。root=半音(0..11), tension=テンションidx, octave=オクターブ差。
struct ChordSlot {
    uint8_t root;
    uint8_t tension;
    int8_t octave;
};
constexpr uint8_t PAD_COUNT = 16;
const ChordSlot& defaultPresetChord(uint8_t pad);

// ---- ノート計算（純粋関数、picotofu.py の base/scale/drumpad_note 相当）----

// 12*(octave+tmpOctave) + transpose + tmp
int baseNote(const MusicState& s, int tmp = 0, int tmpOctave = 0);

// 12*octave + transpose + sum(SCALE_PATTERNS[scaleNum][:val])
// val は度数（0 以上）。範囲はパターン長(21)でクランプ。
int scaleNote(const MusicState& s, int val);

// drumpadOctave により音域が変わる:
//   MIN(0): note = tmp
//   MAX(8): note = 16*7 + tmp
//   その他: note = 16*drumpadOctave + tmp - 12
int drumpadNote(const MusicState& s, int tmp = 0);

// コード構成音を out[] に書き込み、構成音数を返す（最大4）。
// rootNote は baseNote 等で算出済みの実ノート番号。
uint8_t chordNotes(int rootNote, uint8_t tensionIndex, int* out);

// ダイアトニックコード構成音を out[] に書き込み、構成音数を返す。
// val は度数。scaleNote(val) をルートに diatonicPattern から tension を選ぶ。
uint8_t diatonicChordNotes(const MusicState& s, int val, int* out);

}  // namespace music

#endif  // MUSIC_THEORY_H
