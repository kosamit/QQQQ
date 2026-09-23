/*
 * @Description: Immutable adjust helpers for MusicState.
 *   Each returns a new copy; out-of-range changes are ignored (picotofu clamps).
 * @Author: kosamit
 */

#include "music_state.h"
#include "music_theory.h"  // SCALE_COUNT / DIATONIC_COUNT

namespace music {

MusicState withOctaveDelta(const MusicState& s, int direction) {
    MusicState next = s;
    int v = s.octave + direction;
    if (v >= OCTAVE_MIN && v <= OCTAVE_MAX) {
        next.octave = static_cast<int8_t>(v);
    }
    return next;
}

MusicState withDrumpadOctaveDelta(const MusicState& s, int direction) {
    MusicState next = s;
    int v = s.drumpadOctave + direction;
    if (v >= DRUMPAD_OCTAVE_MIN && v <= DRUMPAD_OCTAVE_MAX) {
        next.drumpadOctave = static_cast<int8_t>(v);
    }
    return next;
}

MusicState withTransposeDelta(const MusicState& s, int direction) {
    MusicState next = s;
    int v = s.transpose + direction;
    if (v >= TRANSPOSE_MIN && v <= TRANSPOSE_MAX) {
        next.transpose = static_cast<int8_t>(v);
    }
    return next;
}

MusicState withScaleDelta(const MusicState& s, int direction) {
    MusicState next = s;
    // Python の (n + dir) % len は負でも非負を返す。C++ の % は符号が残るので補正。
    int v = (static_cast<int>(s.scaleNum) + direction) % SCALE_COUNT;
    if (v < 0) v += SCALE_COUNT;
    next.scaleNum = static_cast<uint8_t>(v);
    return next;
}

MusicState withDiatonicDelta(const MusicState& s, int direction) {
    MusicState next = s;
    int v = (static_cast<int>(s.diatonicNum) + direction) % DIATONIC_COUNT;
    if (v < 0) v += DIATONIC_COUNT;
    next.diatonicNum = static_cast<uint8_t>(v);
    return next;
}

}  // namespace music
