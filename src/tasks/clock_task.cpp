/*
 * @Description: Clock task implementation
 * @Author: kosamit
 */

#include "clock_task.h"
#include "../screens/screens.h"
#include "../bluetooth/bluetooth.h"

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
