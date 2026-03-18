/*
 * @Description: Screen drawing and navigation
 * @Author: kosamit
 */

#ifndef SCREENS_H
#define SCREENS_H

#include "../globals.h"

// 画面描画関数
void drawMenuScreen();
void drawDrumPadScreen();
void drawBluetoothSettingsScreen();
void drawAboutScreen();
void switchScreen(ScreenMode newScreen);

// UI部品描画関数
void drawModeButton();
void drawBluetoothStatus();

// ボタン判定関数
bool isTouchInButton(int16_t touchX, int16_t touchY, const ModeButton& btn);

#endif // SCREENS_H
