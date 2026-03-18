/*
 * @Description: MIDI note sending handler
 * @Author: kosamit
 */

#include "midi_handler.h"

// MIDI Note On/Off送信関数（ホールドモード専用）
void sendMidiNotesForChangedCells() {
    if (!bleAdvertising || currentScreen != SCREEN_DRUMPAD) {
        return;
    }

    for (int16_t row = 0; row < GRID_ROWS; row++) {
        for (int16_t col = 0; col < GRID_COLS; col++) {
            int16_t index = row * GRID_COLS + col;
            bool currentState = grid->isCellActive(row, col);

            if (currentState != prevCellState[index]) {
                uint8_t note = grid->getCellMidiNote(row, col);
                uint8_t velocity = 100;

                if (currentState) {
                    MIDI.sendNoteOn(note, velocity, 1);
                    Serial.printf("BLE-MIDI Note On: %d (Row:%d, Col:%d)\n", note, row, col);
                } else {
                    MIDI.sendNoteOff(note, 0, 1);
                    Serial.printf("BLE-MIDI Note Off: %d (Row:%d, Col:%d)\n", note, row, col);
                }

                prevCellState[index] = currentState;
            }
        }
    }
}
