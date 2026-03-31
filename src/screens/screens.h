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
void drawMusicScreen();
void drawBluetoothSettingsScreen();
void drawAboutScreen();
void switchScreen(ScreenMode newScreen);

// 音楽プレイヤー関数
void scanMusicFiles();
void drawMusicFileList();
void playMusicFile(int16_t index);
void stopMusic();

// UI部品描画関数
void drawModeButton();
void drawBluetoothStatus();

// ボタン判定関数
bool isTouchInButton(int16_t touchX, int16_t touchY, const ModeButton& btn);

#endif // SCREENS_H
