/*
 * @Description: Host unit tests for the music theory foundation.
 *   Pure functions only — compile & run on the dev host, no hardware.
 *   Build: see test/run_music_tests.sh
 * @Author: kosamit
 */

#include <cassert>
#include <cstdio>
#include <cstring>

#include "../src/music/music_theory.h"
#include "../src/music/music_state.h"

using namespace music;

static int g_checks = 0;

#define CHECK(cond)                                                    \
    do {                                                               \
        g_checks++;                                                    \
        if (!(cond)) {                                                 \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            return 1;                                                  \
        }                                                              \
    } while (0)

static bool notesEqual(const int* got, uint8_t count, const int* want, uint8_t wantCount) {
    if (count != wantCount) return false;
    for (uint8_t i = 0; i < count; i++) {
        if (got[i] != want[i]) return false;
    }
    return true;
}

int main() {
    MusicState def;  // octave=5, transpose=0, scaleNum=0, diatonicNum=0, drumpadOctave=3

    // ---- noteName ----
    CHECK(std::strcmp(noteName(0), "C") == 0);
    CHECK(std::strcmp(noteName(10), "A#") == 0);
    CHECK(std::strcmp(noteName(12), "C") == 0);  // wrap

    // ---- baseNote ----
    CHECK(baseNote(def, 0, 0) == 60);
    CHECK(baseNote(def, 2, 0) == 62);
    CHECK(baseNote(def, 0, -1) == 48);
    {
        MusicState t = withTransposeDelta(def, 3);
        CHECK(baseNote(t, 0, 0) == 63);
    }

    // ---- scaleNote (Major) ----
    CHECK(scaleNote(def, 0) == 60);
    CHECK(scaleNote(def, 1) == 62);
    CHECK(scaleNote(def, 3) == 65);
    CHECK(scaleNote(def, 7) == 72);
    {
        MusicState minor = def;
        minor.scaleNum = 1;  // Minor
        CHECK(scaleNote(minor, 2) == 63);
    }

    // ---- drumpadNote ----
    CHECK(drumpadNote(def, 0) == 36);   // octave 3: 16*3 - 12
    CHECK(drumpadNote(def, 5) == 41);
    {
        MusicState lo = def; lo.drumpadOctave = DRUMPAD_OCTAVE_MIN;
        CHECK(drumpadNote(lo, 5) == 5);
        MusicState hi = def; hi.drumpadOctave = DRUMPAD_OCTAVE_MAX;
        CHECK(drumpadNote(hi, 5) == 117);  // 16*7 + 5
    }

    // ---- chordNotes ----
    {
        int out[8];
        int wantMaj[] = {60, 64, 67};
        CHECK(notesEqual(out, chordNotes(60, 0, out), wantMaj, 3));
        int wantMaj7[] = {60, 64, 67, 71};
        CHECK(notesEqual(out, chordNotes(60, 4, out), wantMaj7, 4));
        int wantSingle[] = {60};
        CHECK(notesEqual(out, chordNotes(60, 17, out), wantSingle, 1));
    }

    // ---- diatonicChordNotes (Major diatonic, pattern[0..]=0,2,2,0,0,2,9) ----
    {
        int out[8];
        int want0[] = {60, 64, 67};       // deg0: root60, maj
        CHECK(notesEqual(out, diatonicChordNotes(def, 0, out), want0, 3));
        int want1[] = {62, 65, 69};       // deg1: root62, min
        CHECK(notesEqual(out, diatonicChordNotes(def, 1, out), want1, 3));
        int want6[] = {71, 74, 77};       // deg6: root71, m-5
        CHECK(notesEqual(out, diatonicChordNotes(def, 6, out), want6, 3));
    }

    // ---- names ----
    CHECK(std::strcmp(tensionName(10), "m7-5") == 0);
    CHECK(std::strcmp(tensionName(17), "") == 0);
    CHECK(std::strcmp(scaleName(1), "Minor") == 0);
    CHECK(std::strcmp(diatonicName(3), "Minor7th") == 0);

    // ---- defaultPresetChord ----
    {
        const ChordSlot& p0 = defaultPresetChord(0);
        CHECK(p0.root == 0 && p0.tension == 4 && p0.octave == 0);  // Cmaj7
        const ChordSlot& p8 = defaultPresetChord(8);
        CHECK(p8.root == 10 && p8.tension == 16);  // A# s2-5
    }

    // ---- immutable adjust helpers ----
    {
        MusicState up = withOctaveDelta(def, 1);
        CHECK(up.octave == 6 && def.octave == 5);  // original untouched
        MusicState maxed = def; maxed.octave = OCTAVE_MAX;
        CHECK(withOctaveDelta(maxed, 1).octave == OCTAVE_MAX);  // clamp
        // scale wraps: 0 - 1 -> 2 (Dim)
        CHECK(withScaleDelta(def, -1).scaleNum == 2);
        CHECK(withScaleDelta(def, 1).scaleNum == 1);
        // diatonic wraps modulo 4
        CHECK(withDiatonicDelta(def, -1).diatonicNum == 3);
    }

    std::printf("OK: %d checks passed\n", g_checks);
    return 0;
}
