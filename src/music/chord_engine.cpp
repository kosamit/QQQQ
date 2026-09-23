/*
 * @Description: CHORD mode engine implementation (port of picotofu chord logic).
 * @Author: kosamit
 */

#include "chord_engine.h"

#include <stdio.h>
#include <string.h>

namespace music {

namespace {
// Python の n % m は非負を返す。C++ の % は符号が残るので補正する版。
int posMod(int n, int m) {
    int r = n % m;
    if (r < 0) r += m;
    return r;
}
}  // namespace

ChordEngine::ChordEngine() { reset(); }

void ChordEngine::reset() {
    for (uint8_t i = 0; i < PAD_COUNT; i++) {
        chords_[i] = defaultPresetChord(i);
    }
    selectedPad_ = 0;
    state_ = MusicState();
}

const ChordSlot& ChordEngine::chord(uint8_t pad) const {
    static const ChordSlot kEmpty = {0, 0, 0};
    return pad < PAD_COUNT ? chords_[pad] : kEmpty;
}

uint8_t ChordEngine::chordNotesFor(uint8_t pad, int* out) const {
    if (pad >= PAD_COUNT) return 0;
    const ChordSlot& c = chords_[pad];
    int root = baseNote(state_, c.root, c.octave);
    return chordNotes(root, c.tension, out);
}

void ChordEngine::chordName(uint8_t pad, char* out, size_t n) const {
    if (out == nullptr || n == 0) return;
    const ChordSlot& c = chord(pad);
    snprintf(out, n, "%s%s", noteName(c.root), tensionName(c.tension));
}

void ChordEngine::selectPad(uint8_t pad) {
    if (pad < PAD_COUNT) {
        selectedPad_ = pad;
    }
}

void ChordEngine::editRoot(int direction) {
    ChordSlot& c = chords_[selectedPad_];
    c.root = static_cast<uint8_t>(posMod(c.root + direction, NOTE_SIZE));
}

void ChordEngine::editTension(int direction) {
    ChordSlot& c = chords_[selectedPad_];
    c.tension = static_cast<uint8_t>(posMod(c.tension + direction, TENSION_COUNT));
}

void ChordEngine::editOctave(int direction) {
    ChordSlot& c = chords_[selectedPad_];
    int v = c.octave + direction;
    if (v >= CHORD_OCTAVE_MIN && v <= CHORD_OCTAVE_MAX) {
        c.octave = static_cast<int8_t>(v);
    }
}

void ChordEngine::transposeAll(int direction) {
    // picotofu change_chords_transpose を踏襲。root が境界(12 / -1)を跨いだら
    // octave を direction 分だけ桁上げ（元コードのオクターブ補正は常時真=無条件）。
    for (uint8_t i = 0; i < PAD_COUNT; i++) {
        int tmp = chords_[i].root + direction;
        if (tmp == NOTE_SIZE || tmp == -1) {
            chords_[i].octave = static_cast<int8_t>(chords_[i].octave + direction);
        }
        chords_[i].root = static_cast<uint8_t>(posMod(tmp, NOTE_SIZE));
    }
}

void ChordEngine::octaveAll(int direction) {
    for (uint8_t i = 0; i < PAD_COUNT; i++) {
        int v = chords_[i].octave + direction;
        if (v >= CHORD_OCTAVE_MIN && v <= CHORD_OCTAVE_MAX) {
            chords_[i].octave = static_cast<int8_t>(v);
        }
    }
}

void ChordEngine::changeOctave(int direction) {
    state_ = withOctaveDelta(state_, direction);
}

void ChordEngine::changeTranspose(int direction) {
    state_ = withTransposeDelta(state_, direction);
}

}  // namespace music
