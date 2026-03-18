/*
 * @Description: MIDI note sending handler
 * @Author: kosamit
 */

#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include "../globals.h"

// MIDI Note On/Off送信関数（ホールドモード専用）
void sendMidiNotesForChangedCells();

#endif // MIDI_HANDLER_H
