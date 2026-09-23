/*
 * @Description: CHORD mode integration layer implementation.
 * @Author: kosamit
 */

#include "chord_mode.h"

#include "../globals.h"
#include "../neotrellis/neotrellis_handler.h"

// 共有エンジン。
music::ChordEngine chordEngine;

namespace {

// --- 画面色（RGB565）---
constexpr uint16_t COLOR_CELL_BG = 0x2104;      // 通常セル（暗いグレー）
constexpr uint16_t COLOR_PLAYING = 0x001F;      // 演奏中（青）
constexpr uint16_t COLOR_SELECT = 0xFFE0;       // 選択枠（黄）
constexpr uint16_t COLOR_BORDER = 0xFFFF;       // 通常枠（白）
constexpr uint16_t COLOR_TEXT = 0xFFFF;
constexpr uint16_t COLOR_BG = 0x0000;
constexpr uint16_t COLOR_BACK = 0xF800;         // BACK（赤）

// --- NeoTrellis LED色（0xRRGGBB）---
constexpr uint32_t LED_PLAYING = 0x0000FF;
constexpr uint32_t LED_SELECTED = 0x100800;     // 薄いオレンジ
constexpr uint32_t LED_OFF = 0x000000;

// 演奏中フラグと再描画マーク（16パッド）。
bool g_playing[16] = {false};
bool g_dirty[16] = {false};
bool g_infoDirty = false;

// タッチで押下中のパッド（-1 = なし）。
int8_t g_touchHeldPad = -1;

// 現在押下保持中のパッド（タッチ/NeoTrellis 共通, -1 = なし）。
// D-pad がコード編集モードか否かの判定に使う。
int8_t g_heldPad = -1;

void cellOrigin(uint8_t pad, int16_t& x, int16_t& y) {
    int col = pad % 4;
    int row = pad / 4;
    x = CHORD_GRID_X + col * CHORD_CELL_W;
    y = CHORD_GRID_Y + row * CHORD_CELL_H;
}

void drawChordCell(uint8_t pad) {
    int16_t x, y;
    cellOrigin(pad, x, y);

    uint16_t bg = g_playing[pad] ? COLOR_PLAYING : COLOR_CELL_BG;
    gfx->fillRect(x + 1, y + 1, CHORD_CELL_W - 2, CHORD_CELL_H - 2, bg);

    bool selected = (pad == chordEngine.selectedPad());
    uint16_t border = selected ? COLOR_SELECT : COLOR_BORDER;
    gfx->drawRect(x, y, CHORD_CELL_W, CHORD_CELL_H, border);
    if (selected) {
        gfx->drawRect(x + 1, y + 1, CHORD_CELL_W - 2, CHORD_CELL_H - 2, border);
    }

    // ルート音（大, size4=24px/char）を上段、テンション（size2=12px/char）を
    // 下段に中央寄せ。名前を1行に詰めず分けることで、長い名前(A#s2-5等)でも読める。
    // 100x100 セル: root は上寄り(y+22)、tension は下寄り(y+62)。
    const music::ChordSlot& c = chordEngine.chord(pad);
    const char* root = music::noteName(c.root);
    const char* tens = music::tensionName(c.tension);

    gfx->setTextColor(COLOR_TEXT);
    int16_t rootW = (int16_t)strlen(root) * 24;  // 24px/char @ size4
    gfx->setTextSize(4);
    gfx->setCursor(x + (CHORD_CELL_W - rootW) / 2, y + 22);
    gfx->print(root);

    if (tens[0] != '\0') {
        int16_t tW = (int16_t)strlen(tens) * 12;  // 12px/char @ size2
        gfx->setTextSize(2);
        gfx->setCursor(x + (CHORD_CELL_W - tW) / 2, y + 62);
        gfx->print(tens);
    }
}

// --- 情報パネルの配置（150px 幅・25px マージン基準, パネル中心 x=100）---
constexpr int16_t INFO_X = CHORD_MARGIN + 10;   // 35（左マージン+内側余白）
constexpr int16_t PANEL_CX = CHORD_MARGIN + CHORD_INFO_W / 2;  // 100

// テキスト（CHORD/名前/OCT/TR）領域。ここだけを再描画クリアする。
constexpr int16_t INFO_TEXT_Y = CHORD_MARGIN;   // 25
constexpr int16_t INFO_TEXT_H = 120;            // 25..145

// --- D-pad（上下左右ボタン）配置。十字レイアウト ---
constexpr int16_t DPAD_BTN = 42;                // ボタン一辺
constexpr int16_t DPAD_UP_X = PANEL_CX - DPAD_BTN / 2;  // 79
constexpr int16_t DPAD_UP_Y = 158;
constexpr int16_t DPAD_MID_Y = DPAD_UP_Y + DPAD_BTN + 4;  // 204（左右の行）
constexpr int16_t DPAD_L_X = DPAD_UP_X - DPAD_BTN - 4;    // 33
constexpr int16_t DPAD_R_X = DPAD_UP_X + DPAD_BTN + 4;    // 125
constexpr int16_t DPAD_DOWN_Y = DPAD_MID_Y + DPAD_BTN + 4;  // 250

// --- BACK ボタン（最下部・左右に隙間）---
constexpr int16_t BACK_GAP = 15;
constexpr int16_t BACK_X = CHORD_MARGIN + BACK_GAP;         // 40
constexpr int16_t BACK_W = CHORD_INFO_W - BACK_GAP * 2;     // 120
constexpr int16_t BACK_H = 46;
constexpr int16_t BACK_Y = 450 - CHORD_MARGIN - BACK_H;     // 379（下寄せ）

// コントロール（D-pad）識別。
enum ChordCtrl { CTRL_NONE = 0, CTRL_UP, CTRL_DOWN, CTRL_LEFT, CTRL_RIGHT };

// D-pad のエッジ検出用（現在押下中のコントロール）。
int8_t g_ctrlHeld = CTRL_NONE;

void drawChordInfo() {
    // テキスト領域のみクリア（D-pad は消さない）。
    gfx->fillRect(0, INFO_TEXT_Y, CHORD_GRID_X - 6, INFO_TEXT_H, COLOR_BG);

    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(INFO_X, 32);
    gfx->print("CHORD");

    // 選択中コード名（大きめ, size3）。
    char name[music::CHORD_NAME_MAX];
    chordEngine.chordName(chordEngine.selectedPad(), name, sizeof(name));
    gfx->setTextSize(3);
    gfx->setCursor(INFO_X, 62);
    gfx->print(name);

    // OCT / TR（上下の隙間を詰める）。PAD 表示は削除。
    const music::MusicState& s = chordEngine.state();
    gfx->setTextSize(2);
    gfx->setCursor(INFO_X, 100);
    gfx->printf("OCT %d", s.octave);
    gfx->setCursor(INFO_X, 122);
    gfx->printf("TR  %d", s.transpose);
}

// 三角形の矢印を描いた D-pad ボタン。
void drawDpadButton(int16_t bx, int16_t by, ChordCtrl dir) {
    gfx->fillRoundRect(bx, by, DPAD_BTN, DPAD_BTN, 5, COLOR_CELL_BG);
    gfx->drawRoundRect(bx, by, DPAD_BTN, DPAD_BTN, 5, COLOR_BORDER);
    int16_t cx = bx + DPAD_BTN / 2;
    int16_t cy = by + DPAD_BTN / 2;
    const int16_t r = 12;
    switch (dir) {
        case CTRL_UP:
            gfx->fillTriangle(cx, cy - r, cx - r, cy + r, cx + r, cy + r, COLOR_TEXT);
            break;
        case CTRL_DOWN:
            gfx->fillTriangle(cx, cy + r, cx - r, cy - r, cx + r, cy - r, COLOR_TEXT);
            break;
        case CTRL_LEFT:
            gfx->fillTriangle(cx - r, cy, cx + r, cy - r, cx + r, cy + r, COLOR_TEXT);
            break;
        case CTRL_RIGHT:
            gfx->fillTriangle(cx + r, cy, cx - r, cy - r, cx - r, cy + r, COLOR_TEXT);
            break;
        default:
            break;
    }
}

void drawDpad() {
    drawDpadButton(DPAD_UP_X, DPAD_UP_Y, CTRL_UP);
    drawDpadButton(DPAD_L_X, DPAD_MID_Y, CTRL_LEFT);
    drawDpadButton(DPAD_R_X, DPAD_MID_Y, CTRL_RIGHT);
    drawDpadButton(DPAD_UP_X, DPAD_DOWN_Y, CTRL_DOWN);
}

void drawBackButton() {
    gfx->fillRoundRect(BACK_X, BACK_Y, BACK_W, BACK_H, 6, COLOR_BACK);
    gfx->drawRoundRect(BACK_X, BACK_Y, BACK_W, BACK_H, 6, COLOR_BORDER);
    gfx->setTextSize(3);
    gfx->setTextColor(COLOR_TEXT);
    gfx->setCursor(BACK_X + 18, BACK_Y + 12);
    gfx->print("BACK");
}

// 座標 → コントロール種別（D-pad ボタン外は CTRL_NONE）。
ChordCtrl ctrlAt(int16_t x, int16_t y) {
    if (x >= DPAD_UP_X && x < DPAD_UP_X + DPAD_BTN) {
        if (y >= DPAD_UP_Y && y < DPAD_UP_Y + DPAD_BTN) return CTRL_UP;
        if (y >= DPAD_DOWN_Y && y < DPAD_DOWN_Y + DPAD_BTN) return CTRL_DOWN;
    }
    if (y >= DPAD_MID_Y && y < DPAD_MID_Y + DPAD_BTN) {
        if (x >= DPAD_L_X && x < DPAD_L_X + DPAD_BTN) return CTRL_LEFT;
        if (x >= DPAD_R_X && x < DPAD_R_X + DPAD_BTN) return CTRL_RIGHT;
    }
    return CTRL_NONE;
}

// コントロール押下を適用。ドラムパッド保持中は保持コードの root/tension を、
// 通常時はグローバル octave/transpose を編集する。
void applyControl(ChordCtrl c) {
    if (g_heldPad >= 0) {
        uint8_t prevSel = chordEngine.selectedPad();
        chordEngine.selectPad((uint8_t)g_heldPad);
        switch (c) {
            case CTRL_UP:    chordEngine.editRoot(+1); break;   // キー↑
            case CTRL_DOWN:  chordEngine.editRoot(-1); break;   // キー↓
            case CTRL_LEFT:  chordEngine.editTension(-1); break;  // 種類←
            case CTRL_RIGHT: chordEngine.editTension(+1); break;  // 種類→
            default: return;
        }
        g_dirty[g_heldPad] = true;
        if (prevSel != g_heldPad) g_dirty[prevSel] = true;
        g_infoDirty = true;
    } else {
        switch (c) {
            case CTRL_UP:    chordEngine.changeOctave(+1); break;
            case CTRL_DOWN:  chordEngine.changeOctave(-1); break;
            case CTRL_LEFT:  chordEngine.changeTranspose(-1); break;
            case CTRL_RIGHT: chordEngine.changeTranspose(+1); break;
            default: return;
        }
        g_infoDirty = true;
    }
}

void setPadLed(uint8_t pad) {
    if (!neotrellisConnected || trellis == nullptr) return;
    uint32_t color = LED_OFF;
    if (g_playing[pad]) {
        color = LED_PLAYING;
    } else if (pad == chordEngine.selectedPad()) {
        color = LED_SELECTED;
    }
    trellis->pixels.setPixelColor(pad, color);
}

}  // namespace

void drawChordScreen() {
    gfx->fillScreen(COLOR_BG);
    for (uint8_t i = 0; i < 16; i++) {
        g_playing[i] = false;
        g_dirty[i] = false;
        drawChordCell(i);
    }
    drawChordInfo();
    drawDpad();
    drawBackButton();
    g_infoDirty = false;
    chordSyncLEDs();
}

bool chordHandleControls(int16_t* xs, int16_t* ys, uint8_t fingerCount) {
    // 全指を走査し、最初に D-pad ボタンへ乗った指を採用。
    ChordCtrl c = CTRL_NONE;
    for (uint8_t i = 0; i < fingerCount && i < 5; i++) {
        ChordCtrl cc = ctrlAt(xs[i], ys[i]);
        if (cc != CTRL_NONE) {
            c = cc;
            break;
        }
    }
    if (c == CTRL_NONE) {
        g_ctrlHeld = CTRL_NONE;  // どの指もボタン外 → 次の押下を許可
        return false;
    }
    if (c != g_ctrlHeld) {
        g_ctrlHeld = c;
        applyControl(c);
    }
    return true;
}

void chordPadPress(uint8_t pad) {
    if (pad >= 16) return;

    int notes[8];
    uint8_t n = chordEngine.chordNotesFor(pad, notes);
    if (bleAdvertising) {
        for (uint8_t i = 0; i < n; i++) {
            if (notes[i] >= 0 && notes[i] <= 127) {
                MIDI.sendNoteOn((uint8_t)notes[i], chordEngine.state().velocity, 1);
            }
        }
    }

    g_playing[pad] = true;
    g_dirty[pad] = true;
    g_heldPad = (int8_t)pad;
    setPadLed(pad);
    if (neotrellisConnected && trellis != nullptr) trellis->pixels.show();
}

void chordPadRelease(uint8_t pad) {
    if (pad >= 16) return;

    int notes[8];
    uint8_t n = chordEngine.chordNotesFor(pad, notes);
    if (bleAdvertising) {
        for (uint8_t i = 0; i < n; i++) {
            if (notes[i] >= 0 && notes[i] <= 127) {
                MIDI.sendNoteOff((uint8_t)notes[i], 0, 1);
            }
        }
    }

    g_playing[pad] = false;
    g_dirty[pad] = true;
    if (g_heldPad == (int8_t)pad) g_heldPad = -1;

    // 選択パッドを更新（旧選択も再描画対象に）。
    uint8_t prev = chordEngine.selectedPad();
    chordEngine.selectPad(pad);
    if (prev != pad) {
        g_dirty[prev] = true;
    }
    g_infoDirty = true;

    setPadLed(pad);
    if (neotrellisConnected && trellis != nullptr) trellis->pixels.show();
}

int8_t chordPadAt(int16_t x, int16_t y) {
    if (x < CHORD_GRID_X || x >= CHORD_GRID_X + 4 * CHORD_CELL_W) return -1;
    if (y < CHORD_GRID_Y || y >= CHORD_GRID_Y + 4 * CHORD_CELL_H) return -1;
    int col = (x - CHORD_GRID_X) / CHORD_CELL_W;
    int row = (y - CHORD_GRID_Y) / CHORD_CELL_H;
    return (int8_t)(row * 4 + col);
}

void chordHandleTouch(int16_t* xs, int16_t* ys, uint8_t fingerCount) {
    // 全指を走査し、最初にグリッドセルへ乗った指を保持パッドとする。
    // （情報パネルの D-pad を別の指で押しても保持を維持できるようにする）
    int8_t pad = -1;
    for (uint8_t i = 0; i < fingerCount && i < 5; i++) {
        int8_t p = chordPadAt(xs[i], ys[i]);
        if (p >= 0) {
            pad = p;
            break;
        }
    }

    if (pad < 0) {
        // どの指もグリッド上に無い → 保持中パッドを離す。
        if (g_touchHeldPad >= 0) {
            chordPadRelease((uint8_t)g_touchHeldPad);
            g_touchHeldPad = -1;
        }
        return;
    }

    if (g_touchHeldPad != pad) {
        if (g_touchHeldPad >= 0) {
            chordPadRelease((uint8_t)g_touchHeldPad);
        }
        chordPadPress((uint8_t)pad);
        g_touchHeldPad = pad;
    }
}

void chordRedrawDirty() {
    for (uint8_t i = 0; i < 16; i++) {
        if (g_dirty[i]) {
            drawChordCell(i);
            g_dirty[i] = false;
        }
    }
    if (g_infoDirty) {
        drawChordInfo();
        g_infoDirty = false;
    }
}

void chordSyncLEDs() {
    if (!neotrellisConnected || trellis == nullptr) return;
    for (uint8_t i = 0; i < 16; i++) {
        setPadLed(i);
    }
    trellis->pixels.show();
}
