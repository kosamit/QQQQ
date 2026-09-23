/*
 * @Description: Music theory data tables + note-calculation implementation.
 *   Data is a faithful port of picotofu's tension.py / diatonic_chord.py and
 *   the BASE_NOTES / SCALE_* / note helpers in picotofu.py.
 * @Author: kosamit
 */

#include "music_theory.h"

namespace music {

namespace {

// ---- ノート名 ----
const char* const kNoteNames[NOTE_SIZE] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// ---- スケール ----
const char* const kScaleNames[SCALE_COUNT] = {"Major", "Minor", "Dim"};

// SCALE_PATTERNS: 隣接音程（半音）、7度×3オクターブ = 21要素。scaleNote() の累積和用。
const int8_t kScalePatterns[SCALE_COUNT][21] = {
    {2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1},  // Major
    {2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2},  // Minor
    {2, 1, 2, 1, 2, 1, 2, 2, 1, 2, 1, 2, 1, 2, 2, 1, 2, 1, 2, 1, 2},  // Dim
};

// SCALES: 構成音（ルートからの半音差、末尾にオクターブ上を含む）。鍵盤ハイライト用。
const int8_t kScaleMajor[] = {0, 2, 4, 5, 7, 9, 11, 12};
const int8_t kScaleMinor[] = {0, 2, 3, 5, 7, 8, 10, 12};
const int8_t kScaleDim[] = {0, 2, 3, 5, 6, 8, 9, 11, 12};
const int8_t* const kScaleTones[SCALE_COUNT] = {kScaleMajor, kScaleMinor, kScaleDim};
const uint8_t kScaleToneCounts[SCALE_COUNT] = {8, 8, 9};

// ---- テンション（18種、name + fingers）----
const int8_t kTFmaj[] = {0, 4, 7};
const int8_t kTF7[] = {0, 4, 7, 10};
const int8_t kTFmin[] = {0, 3, 7};
const int8_t kTFmin7[] = {0, 3, 7, 10};
const int8_t kTFmaj7[] = {0, 4, 7, 11};
const int8_t kTFmM7[] = {0, 3, 7, 11};
const int8_t kTFsus4[] = {0, 5, 7};
const int8_t kTF7s4[] = {0, 5, 7, 10};
const int8_t kTFdim7[] = {0, 3, 6, 9};
const int8_t kTFm_5[] = {0, 3, 6};
const int8_t kTFm7_5[] = {0, 3, 6, 10};
const int8_t kTFaug[] = {0, 4, 8};
const int8_t kTFadd9[] = {0, 2, 7};
const int8_t kTF6[] = {0, 4, 7, 9};
const int8_t kTFmin6[] = {0, 3, 7, 9};
const int8_t kTFsus2[] = {0, 2, 7};
const int8_t kTFs2_5[] = {0, 2, 6, 9};
const int8_t kTFsingle[] = {0};

struct TensionDef {
    const char* name;
    const int8_t* fingers;
    uint8_t count;
};

const TensionDef kTensions[TENSION_COUNT] = {
    {"maj", kTFmaj, 3},   {"7", kTF7, 4},        {"min", kTFmin, 3},
    {"min7", kTFmin7, 4}, {"maj7", kTFmaj7, 4},  {"mM7", kTFmM7, 4},
    {"sus4", kTFsus4, 3}, {"7s4", kTF7s4, 4},    {"dim7", kTFdim7, 4},
    {"m-5", kTFm_5, 3},   {"m7-5", kTFm7_5, 4},  {"aug", kTFaug, 3},
    {"add9", kTFadd9, 3}, {"6", kTF6, 4},        {"min6", kTFmin6, 4},
    {"sus2", kTFsus2, 3}, {"s2-5", kTFs2_5, 4},  {"", kTFsingle, 1},
};

// ---- ダイアトニックコード（4パターン、各 7度×3オクターブ=21要素）----
// picotofu: pattern = [ ... 7 values ... ] * 3
const uint8_t kDiaMajor[21] = {0, 2, 2, 0, 0, 2, 9, 0, 2, 2, 0, 0, 2, 9, 0, 2, 2, 0, 0, 2, 9};
const uint8_t kDiaMajor7[21] = {4, 3, 3, 4, 1, 3, 10, 4, 3, 3, 4, 1, 3, 10, 4, 3, 3, 4, 1, 3, 10};
const uint8_t kDiaMinor[21] = {2, 9, 0, 2, 2, 0, 0, 2, 9, 0, 2, 2, 0, 0, 2, 9, 0, 2, 2, 0, 0};
const uint8_t kDiaMinor7[21] = {3, 10, 4, 3, 3, 4, 1, 3, 10, 4, 3, 3, 4, 1, 3, 10, 4, 3, 3, 4, 1};

const char* const kDiatonicNames[DIATONIC_COUNT] = {"Major", "Major7th", "Minor", "Minor7th"};
const uint8_t* const kDiatonicPatterns[DIATONIC_COUNT] = {kDiaMajor, kDiaMajor7, kDiaMinor, kDiaMinor7};

// ---- プリセットコード（sd/chords.csv 由来）----
const ChordSlot kDefaultPreset[PAD_COUNT] = {
    {0, 4, 0},  {2, 10, 0}, {2, 3, 0}, {4, 10, 0}, {4, 3, 0}, {5, 3, 0},
    {5, 4, 0},  {7, 10, 0}, {10, 16, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0},  {0, 0, 0},  {0, 0, 0}, {0, 0, 0},
};

constexpr uint8_t kScalePatternLen = 21;

}  // namespace

// ---- アクセサ ----

const char* noteName(uint8_t semitone) {
    return kNoteNames[semitone % NOTE_SIZE];
}

const char* scaleName(uint8_t index) {
    return index < SCALE_COUNT ? kScaleNames[index] : "";
}

const int8_t* scaleTones(uint8_t index, uint8_t& count) {
    if (index >= SCALE_COUNT) {
        count = 0;
        return nullptr;
    }
    count = kScaleToneCounts[index];
    return kScaleTones[index];
}

const char* tensionName(uint8_t index) {
    return index < TENSION_COUNT ? kTensions[index].name : "";
}

const int8_t* tensionFingers(uint8_t index, uint8_t& count) {
    if (index >= TENSION_COUNT) {
        count = 0;
        return nullptr;
    }
    count = kTensions[index].count;
    return kTensions[index].fingers;
}

const char* diatonicName(uint8_t index) {
    return index < DIATONIC_COUNT ? kDiatonicNames[index] : "";
}

const uint8_t* diatonicPattern(uint8_t index) {
    return index < DIATONIC_COUNT ? kDiatonicPatterns[index] : nullptr;
}

const ChordSlot& defaultPresetChord(uint8_t pad) {
    static const ChordSlot kEmpty = {0, 0, 0};
    return pad < PAD_COUNT ? kDefaultPreset[pad] : kEmpty;
}

// ---- ノート計算 ----

int baseNote(const MusicState& s, int tmp, int tmpOctave) {
    return 12 * (s.octave + tmpOctave) + s.transpose + tmp;
}

int scaleNote(const MusicState& s, int val) {
    uint8_t scaleIdx = s.scaleNum < SCALE_COUNT ? s.scaleNum : 0;
    if (val < 0) val = 0;
    if (val > kScalePatternLen) val = kScalePatternLen;
    int sum = 0;
    for (int i = 0; i < val; i++) {
        sum += kScalePatterns[scaleIdx][i];
    }
    return 12 * s.octave + s.transpose + sum;
}

int drumpadNote(const MusicState& s, int tmp) {
    if (s.drumpadOctave == DRUMPAD_OCTAVE_MIN) {
        return tmp;
    }
    if (s.drumpadOctave == DRUMPAD_OCTAVE_MAX) {
        return 16 * 7 + tmp;
    }
    return 16 * s.drumpadOctave + tmp - 12;
}

uint8_t chordNotes(int rootNote, uint8_t tensionIndex, int* out) {
    uint8_t count = 0;
    const int8_t* fingers = tensionFingers(tensionIndex, count);
    if (fingers == nullptr) return 0;
    for (uint8_t i = 0; i < count; i++) {
        out[i] = rootNote + fingers[i];
    }
    return count;
}

uint8_t diatonicChordNotes(const MusicState& s, int val, int* out) {
    const uint8_t* pattern = diatonicPattern(s.diatonicNum);
    if (pattern == nullptr) return 0;
    int degree = val;
    if (degree < 0) degree = 0;
    if (degree >= kScalePatternLen) degree = kScalePatternLen - 1;
    int root = scaleNote(s, degree);
    uint8_t tensionIndex = pattern[degree];
    return chordNotes(root, tensionIndex, out);
}

}  // namespace music
