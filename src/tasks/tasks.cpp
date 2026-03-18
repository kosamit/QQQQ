/*
 * @Description: FreeRTOS task implementations
 * @Author: kosamit
 */

#include "tasks.h"
#include "../screens/screens.h"
#include "../bluetooth/bluetooth.h"
#include "../midi/midi_handler.h"
#include "../debug/touch_info.h"

// タッチ処理タスク（割り込み駆動・マルチタッチ対応・高速化版）
void touchTask(void* parameter)
{
    Serial.println("タッチタスク開始（高速化モード）");

    while (true) {
        uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (notificationValue > 0) {
            vTaskDelay(pdMS_TO_TICKS(3));

            Update_Touch_Info();

            TouchEvent event;
            event.finger_count = global_touch_info.fingers_number;

            for (int i = 0; i < 5; i++) {
                event.x[i] = global_touch_info.touch_x[i];
                event.y[i] = global_touch_info.touch_y[i];
            }

            xQueueSend(touchEventQueue, &event, 0);

            CST226SE->IIC_Interrupt_Flag = false;
        }
    }
}

// ディスプレイ更新タスク（画面ごとに完全分離）
void displayTask(void* parameter)
{
    Serial.println("ディスプレイタスク開始（画面分離モード）");

    TouchEvent event;

    static bool menuLastTouched = false;
    static bool drumpadLastButtonTouched = false;
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

                        for (int i = 0; i < 3; i++) {
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

// 時計更新タスク
void clockTask(void* parameter)
{
    Serial.println("時計タスク開始");

    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t updateInterval = pdMS_TO_TICKS(1000);

    while (true) {
        if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            if (currentScreen == SCREEN_DRUMPAD) {
                GFX_Print_Time_Info_Loop();
            } else if (currentScreen == SCREEN_BLUETOOTH) {
                unsigned long currentMillis = millis();
                if (currentMillis - lastBleStatusUpdate > 1000) {
                    drawBluetoothSettingsScreen();
                    lastBleStatusUpdate = currentMillis;
                }
            }

            xSemaphoreGive(displayMutex);
        }

        vTaskDelayUntil(&lastWakeTime, updateInterval);
    }
}
