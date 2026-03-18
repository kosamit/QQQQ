/*
 * @Description: Screen drawing and navigation
 * @Author: kosamit
 */

#include "screens.h"

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

    for (int i = 0; i < 3; i++) {
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

// 画面切り替え
void switchScreen(ScreenMode newScreen) {
    if (currentScreen == SCREEN_DRUMPAD && newScreen != SCREEN_DRUMPAD) {
        Serial.println("Clearing drumpad state...");

        grid->clearAllActive();
        grid->clearAllTouches();

        for (int i = 0; i < GRID_ROWS * GRID_COLS; i++) {
            prevCellState[i] = false;
        }
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
