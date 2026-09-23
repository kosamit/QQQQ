/*
 * @Description: Host unit tests for ChordEngine (CHORD mode logic).
 * @Author: kosamit
 */

#include <cstdio>
#include <cstring>

#include "../src/music/chord_engine.h"

using namespace music;

static int g_checks = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        g_checks++;                                                     \
        if (!(cond)) {                                                  \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            return 1;                                                   \
        }                                                               \
    } while (0)

static bool notesEqual(const int* got, uint8_t count, const int* want, uint8_t wantCount) {
    if (count != wantCount) return false;
    for (uint8_t i = 0; i < count; i++) {
        if (got[i] != want[i]) return false;
    }
    return true;
}

int main() {
    int out[8];
    char name[CHORD_NAME_MAX];

    // ---- default preset voicing + names ----
    {
        ChordEngine e;
        int w0[] = {60, 64, 67, 71};  // pad0 Cmaj7
        CHECK(notesEqual(out, e.chordNotesFor(0, out), w0, 4));
        e.chordName(0, name, sizeof(name));
        CHECK(std::strcmp(name, "Cmaj7") == 0);

        int w1[] = {62, 65, 68, 72};  // pad1 Dm7-5
        CHECK(notesEqual(out, e.chordNotesFor(1, out), w1, 4));
        e.chordName(1, name, sizeof(name));
        CHECK(std::strcmp(name, "Dm7-5") == 0);

        int w8[] = {70, 72, 76, 79};  // pad8 A# s2-5
        CHECK(notesEqual(out, e.chordNotesFor(8, out), w8, 4));
        e.chordName(8, name, sizeof(name));
        CHECK(std::strcmp(name, "A#s2-5") == 0);

        int w9[] = {60, 64, 67};  // pad9 Cmaj (empty slot {0,0,0})
        CHECK(notesEqual(out, e.chordNotesFor(9, out), w9, 3));
        e.chordName(9, name, sizeof(name));
        CHECK(std::strcmp(name, "Cmaj") == 0);
    }

    // ---- selectPad ----
    {
        ChordEngine e;
        CHECK(e.selectedPad() == 0);
        e.selectPad(5);
        CHECK(e.selectedPad() == 5);
        e.selectPad(99);  // out of range -> unchanged
        CHECK(e.selectedPad() == 5);
    }

    // ---- editRoot / editTension / editOctave (selected pad) ----
    {
        ChordEngine e;
        e.selectPad(9);  // C maj {0,0,0}
        e.editRoot(1);
        CHECK(e.chord(9).root == 1);  // C#
        e.editRoot(-1);
        e.editRoot(-1);
        CHECK(e.chord(9).root == 11);  // wrap to B

        e.selectPad(9);
        e.editTension(-1);
        CHECK(e.chord(9).tension == 17);  // wrap to single ""

        e.editOctave(1);
        CHECK(e.chord(9).octave == 1);
        e.editOctave(1);
        e.editOctave(1);
        e.editOctave(1);  // clamp at +3
        CHECK(e.chord(9).octave == 3);
        for (int i = 0; i < 10; i++) e.editOctave(-1);  // clamp at -3
        CHECK(e.chord(9).octave == -3);
    }

    // ---- octaveAll shifts every pad's voicing ----
    {
        ChordEngine e;
        e.octaveAll(1);
        CHECK(e.chord(0).octave == 1);
        int w[] = {72, 76, 79, 83};  // pad0 Cmaj7 an octave up
        CHECK(notesEqual(out, e.chordNotesFor(0, out), w, 4));
    }

    // ---- transposeAll with octave carry at boundary ----
    {
        ChordEngine e;
        e.selectPad(0);
        e.editRoot(-1);  // pad0 root 0 -> 11
        CHECK(e.chord(0).root == 11);
        e.transposeAll(1);  // 11 + 1 == 12 -> carry octave, root wraps to 0
        CHECK(e.chord(0).root == 0);
        CHECK(e.chord(0).octave == 1);
        // pad1 root 2 -> 3, no carry
        CHECK(e.chord(1).root == 3 && e.chord(1).octave == 0);
    }

    // ---- global changeOctave / changeTranspose affect voicing ----
    {
        ChordEngine e;
        e.changeOctave(1);  // octave 5 -> 6
        int w[] = {72, 76, 79, 83};  // pad0 Cmaj7 with global octave 6
        CHECK(notesEqual(out, e.chordNotesFor(0, out), w, 4));
        e.changeTranspose(2);  // +2 semitones
        int wt[] = {74, 78, 81, 85};
        CHECK(notesEqual(out, e.chordNotesFor(0, out), wt, 4));
    }

    // ---- reset restores defaults ----
    {
        ChordEngine e;
        e.selectPad(3);
        e.editRoot(5);
        e.changeOctave(1);
        e.reset();
        CHECK(e.selectedPad() == 0);
        CHECK(e.chord(0).root == 0 && e.chord(0).tension == 4);
        int w0[] = {60, 64, 67, 71};
        CHECK(notesEqual(out, e.chordNotesFor(0, out), w0, 4));
    }

    std::printf("OK: %d checks passed\n", g_checks);
    return 0;
}
