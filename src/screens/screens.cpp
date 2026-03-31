/*
 * @Description: Screen drawing and navigation
 * @Author: kosamit
 */

#include "screens.h"
#include "../neotrellis/neotrellis_handler.h"
#include <SD.h>

// モード切り替えボタンを描画
void drawModeButton() {
    uint16_t bgColor = (currentTouchMode == TOUCH_MODE_TOGGLE) ? 0x2104 : 0x8C51;
    gfx->fillRect(modeButton.x, modeButton.y, modeButton.width, modeButton.height, bgColor);

    gfx->drawRect(modeButton.x, modeButton.y, modeButton.width, modeButton.height, WHITE);
    gfx->drawRect(modeButton.x + 1, modeButton.y + 1, modeButton.width - 2, modeButton.height - 2, WHITE);

    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);

    const char* label = (currentTouchMode == TOUCH_MODE_TOGGLE) ? modeButton.label_toggle : modeButton.label_hold;
    int16_t textWidth = strlen(label) * 12;
    int16_t textX = modeButton.x + (modeButton.width - textWidth) / 2;
    int16_t textY = modeButton.y + (modeButton.height - 16) / 2;

    gfx->setCursor(textX, textY);
    gfx->print(label);
}

// ボタンがタッチされたか判定
bool isTouchInButton(int16_t touchX, int16_t touchY, const ModeButton& btn) {
    return (touchX >= btn.x && touchX < btn.x + btn.width &&
            touchY >= btn.y && touchY < btn.y + btn.height);
}

// Bluetooth接続状態を描画
void drawBluetoothStatus() {
    int16_t statusX = bleButton.x;
    int16_t statusY = bleButton.y;
    int16_t statusWidth = bleButton.width;
    int16_t statusHeight = bleButton.height;

    uint16_t bgColor;
    const char* statusText;

    if (bleConnected) {
        bgColor = 0x07E0;
        statusText = "CONNECTED";
    } else if (bleAdvertising) {
        bgColor = 0xFFE0;
        statusText = "WAITING";
    } else {
        bgColor = 0xF800;
        statusText = "BLE OFF";
    }

    gfx->fillRect(statusX, statusY, statusWidth, statusHeight, bgColor);
    gfx->drawRect(statusX, statusY, statusWidth, statusHeight, WHITE);
    gfx->drawRect(statusX + 1, statusY + 1, statusWidth - 2, statusHeight - 2, WHITE);

    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);

    int16_t textWidth = strlen(statusText) * 6;
    int16_t textX = statusX + (statusWidth - textWidth) / 2;
    int16_t textY = statusY + (statusHeight - 8) / 2;

    gfx->setCursor(textX, textY);
    gfx->print(statusText);

    if (bleAdvertising && !bleConnected) {
        gfx->fillRect(0, 200, 480, 22, BLACK);
        gfx->setTextSize(2);
        gfx->setTextColor(WHITE);
        gfx->setCursor(10, 200);
        gfx->print(String("BLE: ") + DEVICE_NAME);

        gfx->setTextSize(1);
        gfx->setCursor(350, 200);
        gfx->print("Tap to stop");
    }
}

// メニュー画面描画
void drawMenuScreen() {
    gfx->fillScreen(BLACK);

    gfx->setTextSize(3);
    gfx->setTextColor(WHITE);
    gfx->setCursor(160, 5);
    gfx->print(DEVICE_NAME);

    for (int i = 0; i < 4; i++) {
        MenuItem& item = menuItems[i];

        uint16_t color = 0x4208;
        gfx->fillRoundRect(item.x, item.y, item.width, item.height, 5, color);
        gfx->drawRoundRect(item.x, item.y, item.width, item.height, 5, WHITE);
        gfx->drawRoundRect(item.x+1, item.y+1, item.width-2, item.height-2, 5, WHITE);

        gfx->setTextSize(2);
        gfx->setTextColor(WHITE);
        int16_t textWidth = strlen(item.label) * 12;
        gfx->setCursor(item.x + (item.width - textWidth) / 2, item.y + 18);
        gfx->print(item.label);
    }
}

// ドラムパッド画面描画
void drawDrumPadScreen() {
    gfx->fillScreen(WHITE);

    grid->draw();
    drawModeButton();

    gfx->fillRoundRect(10, 172, 100, 40, 5, 0xF800);
    gfx->drawRoundRect(10, 172, 100, 40, 5, WHITE);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(30, 185);
    gfx->print("BACK");
}

// Bluetooth設定画面描画
void drawBluetoothSettingsScreen() {
    gfx->fillScreen(BLACK);

    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(10, 10);
    gfx->print("Bluetooth Settings");

    int16_t btnX = 90;
    int16_t btnY = 50;
    int16_t btnW = 300;
    int16_t btnH = 60;

    uint16_t bgColor;
    const char* statusText;

    if (bleConnected) {
        bgColor = 0x07E0;
        statusText = "CONNECTED";
    } else if (bleAdvertising) {
        bgColor = 0xFFE0;
        statusText = "WAITING...";
    } else {
        bgColor = 0xF800;
        statusText = "BLE OFF";
    }

    gfx->fillRoundRect(btnX, btnY, btnW, btnH, 5, bgColor);
    gfx->drawRoundRect(btnX, btnY, btnW, btnH, 5, WHITE);
    gfx->setTextSize(3);
    gfx->setTextColor(BLACK);
    int16_t textWidth = strlen(statusText) * 18;
    gfx->setCursor(btnX + (btnW - textWidth) / 2, btnY + 20);
    gfx->print(statusText);

    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);
    gfx->setCursor(btnX, btnY + btnH + 10);
    gfx->print(String("Device Name: ") + DEVICE_NAME);

    gfx->setCursor(btnX, btnY + btnH + 25);
    gfx->print("Tap button to toggle BLE");

    gfx->fillRoundRect(10, 172, 100, 40, 5, 0xF800);
    gfx->drawRoundRect(10, 172, 100, 40, 5, WHITE);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(30, 185);
    gfx->print("BACK");
}

// About画面描画
void drawAboutScreen() {
    gfx->fillScreen(BLACK);

    gfx->setTextSize(3);
    gfx->setTextColor(WHITE);
    gfx->setCursor(180, 10);
    gfx->print("ABOUT");

    gfx->setTextSize(2);
    gfx->setCursor(10, 50);
    gfx->print(DEVICE_NAME);

    gfx->setTextSize(1);
    gfx->setCursor(10, 75);
    gfx->print("BLE-MIDI Controller");

    gfx->setCursor(10, 90);
    gfx->print("Firmware: v1.0");

    gfx->setCursor(10, 105);
    gfx->print("Board: ESP32-S3");

    gfx->setCursor(10, 120);
    gfx->print("Display: 480x222");

    gfx->setCursor(10, 135);
    gfx->print("Grid: 4x4 Touch Pads");

    gfx->setCursor(10, 150);
    gfx->print("MIDI Notes: 60-75 (C4-D#5)");

    gfx->fillRoundRect(10, 172, 100, 40, 5, 0xF800);
    gfx->drawRoundRect(10, 172, 100, 40, 5, WHITE);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(30, 185);
    gfx->print("BACK");
}

// SDカードからオーディオファイルをスキャン
void scanMusicFiles() {
    musicFileCount = 0;
    musicScrollOffset = 0;
    musicSelectedIndex = -1;

    if (!SD_Initialization_Flag) {
        return;
    }

    File root = SD.open("/");
    if (!root) {
        return;
    }

    while (musicFileCount < MUSIC_MAX_FILES) {
        File entry = root.openNextFile();
        if (!entry) break;

        if (!entry.isDirectory()) {
            String name = entry.name();
            name.toLowerCase();
            if (name.endsWith(".mp3") || name.endsWith(".wav") || name.endsWith(".flac")) {
                // ファイル名を保存（"/"付き）
                snprintf(musicFiles[musicFileCount], 64, "/%s", entry.name());
                musicFileCount++;
            }
        }
        entry.close();
    }
    root.close();

    Serial.printf("Found %d audio files\n", musicFileCount);
}

// ファイルリストを描画
void drawMusicFileList() {
    // リスト領域をクリア
    int16_t listHeight = 170 - MUSIC_LIST_Y_START;
    gfx->fillRect(0, MUSIC_LIST_Y_START, 480, listHeight, BLACK);

    if (!SD_Initialization_Flag) {
        gfx->setTextSize(2);
        gfx->setTextColor(0xF800);
        gfx->setCursor(100, 80);
        gfx->print("SD card not found");
        return;
    }

    if (musicFileCount == 0) {
        gfx->setTextSize(2);
        gfx->setTextColor(0xFFE0);
        gfx->setCursor(100, 80);
        gfx->print("No audio files");
        return;
    }

    int16_t visibleItems = listHeight / MUSIC_ITEM_HEIGHT;
    gfx->setTextSize(2);

    for (int16_t i = 0; i < visibleItems && (i + musicScrollOffset) < musicFileCount; i++) {
        int16_t fileIdx = i + musicScrollOffset;
        int16_t y = MUSIC_LIST_Y_START + i * MUSIC_ITEM_HEIGHT;

        // 選択中（再生中）のハイライト
        if (fileIdx == musicSelectedIndex && musicIsPlaying) {
            gfx->fillRect(10, y, 460, MUSIC_ITEM_HEIGHT - 2, 0x0320);
        } else if (fileIdx == musicSelectedIndex) {
            gfx->fillRect(10, y, 460, MUSIC_ITEM_HEIGHT - 2, 0x2104);
        }

        gfx->setTextColor(WHITE);
        gfx->setCursor(15, y + 5);

        // 再生中マーク
        if (fileIdx == musicSelectedIndex && musicIsPlaying) {
            gfx->print("> ");
        } else {
            gfx->print("  ");
        }

        // ファイル名（"/"を除いて表示、長すぎる場合はカット）
        const char* fname = musicFiles[fileIdx] + 1; // skip leading "/"
        char displayName[32];
        strncpy(displayName, fname, 31);
        displayName[31] = '\0';
        gfx->print(displayName);
    }
}

// 音楽画面描画
void drawMusicScreen() {
    gfx->fillScreen(BLACK);

    // タイトル
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(10, 8);
    gfx->print("Music Player");

    // 停止ボタン
    if (musicIsPlaying) {
        gfx->fillRoundRect(350, 5, 120, 22, 3, 0xF800);
        gfx->drawRoundRect(350, 5, 120, 22, 3, WHITE);
        gfx->setTextSize(1);
        gfx->setTextColor(WHITE);
        gfx->setCursor(385, 12);
        gfx->print("STOP");
    }

    // ファイルリスト
    scanMusicFiles();
    drawMusicFileList();

    // 戻るボタン
    gfx->fillRoundRect(10, 172, 100, 40, 5, 0xF800);
    gfx->drawRoundRect(10, 172, 100, 40, 5, WHITE);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(30, 185);
    gfx->print("BACK");
}

// ファイル再生
void playMusicFile(int16_t index) {
    if (index < 0 || index >= musicFileCount || !SD_Initialization_Flag) return;

    audio.connecttoSD(musicFiles[index]);
    musicSelectedIndex = index;
    musicIsPlaying = true;

    Serial.printf("Playing: %s\n", musicFiles[index]);
}

// 再生停止
void stopMusic() {
    audio.stopSong();
    musicIsPlaying = false;
    Serial.println("Music stopped");
}

// 画面切り替え
void switchScreen(ScreenMode newScreen) {
    if (currentScreen == SCREEN_DRUMPAD && newScreen != SCREEN_DRUMPAD) {
        Serial.println("Clearing drumpad state...");

        grid->clearAllActive();
        grid->clearAllTouches();

        for (int i = 0; i < GRID_ROWS * GRID_COLS; i++) {
            prevCellState[i] = false;
        }

        // NeoTrellis LEDもクリア
        clearNeoTrellisLEDs();
    }

    currentScreen = newScreen;

    Serial.print("Switching to screen: ");
    switch(newScreen) {
        case SCREEN_MENU:
            Serial.println("MENU");
            drawMenuScreen();
            break;
        case SCREEN_DRUMPAD:
            Serial.println("DRUMPAD");
            drawDrumPadScreen();
            syncNeoTrellisLEDs();
            break;
        case SCREEN_MUSIC:
            Serial.println("MUSIC");
            drawMusicScreen();
            break;
        case SCREEN_BLUETOOTH:
            Serial.println("BLUETOOTH");
            drawBluetoothSettingsScreen();
            break;
        case SCREEN_ABOUT:
            Serial.println("ABOUT");
            drawAboutScreen();
            break;
    }
}
