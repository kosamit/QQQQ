/*
 * @Description: Display task implementation
 * @Author: kosamit
 */

#include "display_task.h"
#include "../screens/screens.h"
#include "../bluetooth/bluetooth.h"
#include "../midi/midi_handler.h"

// ディスプレイ更新タスク（画面ごとに完全分離）
void displayTask(void* parameter)
{
    Serial.println("ディスプレイタスク開始（画面分離モード）");

    TouchEvent event;

    static bool menuLastTouched = false;
    static bool drumpadLastButtonTouched = false;
    static bool musicLastTouched = false;
    static bool bluetoothLastTouched = false;
    static bool aboutLastTouched = false;

    while (true) {
        if (xQueueReceive(touchEventQueue, &event, pdMS_TO_TICKS(1)) == pdTRUE) {
            if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(5)) == pdTRUE) {

                ScreenMode screen = currentScreen;

                // ========================================
                // メニュー画面の処理
                // ========================================
                if (screen == SCREEN_MENU) {
                    if (event.finger_count > 0 && !menuLastTouched) {
                        int16_t touchX = event.x[0];
                        int16_t touchY = event.y[0];

                        for (int i = 0; i < 4; i++) {
                            MenuItem& item = menuItems[i];
                            if (touchX >= item.x && touchX < item.x + item.width &&
                                touchY >= item.y && touchY < item.y + item.height) {
                                switchScreen(item.targetScreen);
                                menuLastTouched = true;
                                break;
                            }
                        }
                    }

                    if (event.finger_count == 0) {
                        menuLastTouched = false;
                    }
                }

                // ========================================
                // ドラムパッド画面の処理（完全独立）
                // ========================================
                else if (screen == SCREEN_DRUMPAD) {
                    bool buttonHandled = false;
                    bool currentButtonTouched = false;

                    if (event.finger_count > 0) {
                        int16_t touchX = event.x[0];
                        int16_t touchY = event.y[0];

                        // 戻るボタン
                        if (touchX >= 10 && touchX < 110 && touchY >= 172 && touchY < 212) {
                            currentButtonTouched = true;
                            if (!drumpadLastButtonTouched) {
                                switchScreen(SCREEN_MENU);
                                drumpadLastButtonTouched = true;
                                xSemaphoreGive(displayMutex);
                                continue;
                            }
                            buttonHandled = true;
                        }
                        // モード切り替えボタン
                        else if (isTouchInButton(touchX, touchY, modeButton)) {
                            currentButtonTouched = true;

                            if (!drumpadLastButtonTouched) {
                                if (currentTouchMode == TOUCH_MODE_TOGGLE) {
                                    currentTouchMode = TOUCH_MODE_HOLD;
                                } else {
                                    currentTouchMode = TOUCH_MODE_TOGGLE;
                                }

                                grid->setTouchMode(currentTouchMode);
                                drawModeButton();
                            }

                            buttonHandled = true;
                        }
                    }

                    drumpadLastButtonTouched = currentButtonTouched;

                    if (!buttonHandled && screen == SCREEN_DRUMPAD) {
                        grid->handleMultiTouch(event.x, event.y, event.finger_count);

                        if (currentTouchMode == TOUCH_MODE_HOLD) {
                            sendMidiNotesForChangedCells();
                        }

                        grid->redrawChangedCells();
                    }
                }

                // ========================================
                // 音楽プレイヤー画面の処理
                // ========================================
                else if (screen == SCREEN_MUSIC) {
                    if (event.finger_count > 0 && !musicLastTouched) {
                        int16_t touchX = event.x[0];
                        int16_t touchY = event.y[0];

                        // 戻るボタン
                        if (touchX >= 10 && touchX < 110 && touchY >= 172 && touchY < 212) {
                            switchScreen(SCREEN_MENU);
                            musicLastTouched = true;
                        }
                        // 停止ボタン
                        else if (musicIsPlaying && touchX >= 350 && touchX < 470 && touchY >= 5 && touchY < 27) {
                            stopMusic();
                            drawMusicScreen();
                            musicLastTouched = true;
                        }
                        // ファイルリストのタッチ
                        else if (touchY >= MUSIC_LIST_Y_START && touchY < 170 && touchX >= 10 && touchX < 470) {
                            int16_t itemIndex = (touchY - MUSIC_LIST_Y_START) / MUSIC_ITEM_HEIGHT + musicScrollOffset;
                            if (itemIndex >= 0 && itemIndex < musicFileCount) {
                                playMusicFile(itemIndex);
                                drawMusicFileList();
                                // 停止ボタンも更新
                                gfx->fillRoundRect(350, 5, 120, 22, 3, 0xF800);
                                gfx->drawRoundRect(350, 5, 120, 22, 3, WHITE);
                                gfx->setTextSize(1);
                                gfx->setTextColor(WHITE);
                                gfx->setCursor(385, 12);
                                gfx->print("STOP");
                            }
                            musicLastTouched = true;
                        }
                    }

                    if (event.finger_count == 0) {
                        musicLastTouched = false;
                    }
                }

                // ========================================
                // Bluetooth設定画面の処理
                // ========================================
                else if (screen == SCREEN_BLUETOOTH) {
                    if (event.finger_count > 0 && !bluetoothLastTouched) {
                        int16_t touchX = event.x[0];
                        int16_t touchY = event.y[0];

                        if (touchX >= 10 && touchX < 110 && touchY >= 172 && touchY < 212) {
                            switchScreen(SCREEN_MENU);
                            bluetoothLastTouched = true;
                        }
                        else if (touchX >= 90 && touchX < 390 && touchY >= 50 && touchY < 110) {
                            toggleBluetooth();
                            drawBluetoothSettingsScreen();
                            bluetoothLastTouched = true;
                        }
                    }

                    if (event.finger_count == 0) {
                        bluetoothLastTouched = false;
                    }
                }

                // ========================================
                // About画面の処理
                // ========================================
                else if (screen == SCREEN_ABOUT) {
                    if (event.finger_count > 0 && !aboutLastTouched) {
                        int16_t touchX = event.x[0];
                        int16_t touchY = event.y[0];

                        if (touchX >= 10 && touchX < 110 && touchY >= 172 && touchY < 212) {
                            switchScreen(SCREEN_MENU);
                            aboutLastTouched = true;
                        }
                    }

                    if (event.finger_count == 0) {
                        aboutLastTouched = false;
                    }
                }

                xSemaphoreGive(displayMutex);
            }
        }
    }
}
