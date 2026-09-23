/*
 * @Description: CHORD mode engine (picotofu chord mode logic).
 *   Owns the 16-pad chord assignment, the selected pad, and the global
 *   octave/transpose used for voicing. Note computation stays pure via
 *   music_theory; MIDI output and grid drawing live in the integration layer.
 *   No Arduino/hardware dependency -> unit-testable on the host.
 * @Author: kosamit
 */

#ifndef CHORD_ENGINE_H
#define CHORD_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#include "music_state.h"
#include "music_theory.h"

namespace music {

// コード名の最大長（"A#min7" 等 + 終端）。
constexpr size_t CHORD_NAME_MAX = 12;

// 選択パッドのコードオクターブ差の範囲（picotofu: -3..3）。
constexpr int8_t CHORD_OCTAVE_MIN = -3;
constexpr int8_t CHORD_OCTAVE_MAX = 3;

class ChordEngine {
   public:
    ChordEngine();  // デフォルトプリセットで初期化

    void reset();  // プリセット再ロード + 選択パッドを0に

    // --- 参照 ---
    const ChordSlot& chord(uint8_t pad) const;
    uint8_t selectedPad() const { return selectedPad_; }
    const MusicState& state() const { return state_; }

    // 押下したパッドのコード構成音を out[] に書き込み、構成音数を返す（最大4）。
    uint8_t chordNotesFor(uint8_t pad, int* out) const;

    // コード名（"Cmaj7" 等）を out に書き込む（noteName + tensionName）。
    void chordName(uint8_t pad, char* out, size_t n) const;

    // --- 選択・編集（選択パッドに作用）---
    void selectPad(uint8_t pad);
    void editRoot(int direction);     // root を %12 で増減
    void editTension(int direction);  // tension を %TENSION_COUNT で増減
    void editOctave(int direction);   // octave を [-3,3] でクランプ増減

    // --- 全16パッド一括 ---
    void transposeAll(int direction);  // 全 root を増減（境界でオクターブ桁上げ）
    void octaveAll(int direction);     // 全 octave を [-3,3] で増減

    // --- グローバル octave / transpose（コントローラ操作）---
    void changeOctave(int direction);
    void changeTranspose(int direction);

   private:
    ChordSlot chords_[PAD_COUNT];
    uint8_t selectedPad_ = 0;
    MusicState state_;
};

}  // namespace music

#endif  // CHORD_ENGINE_H
