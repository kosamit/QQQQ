/*
 * @Description: NeoTrellis polling task
 * @Author: kosamit
 */

#include "neotrellis_task.h"
#include "../neotrellis/neotrellis_handler.h"

void neotrellisTask(void* parameter)
{
    Serial.println("NeoTrellisタスク開始");

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 20ms間隔でポーリング

    while (true) {
        if (neotrellisConnected && trellis != nullptr) {
            // ボタンイベント読み取り（コールバックが呼ばれる）
            trellis->read();

            // ディスプレイ更新（displayMutex取得）
            if (currentScreen == SCREEN_DRUMPAD && grid != nullptr) {
                if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                    grid->redrawChangedCells();
                    xSemaphoreGive(displayMutex);
                }
            }

            // NeoTrellis LEDをGrid状態に同期
            syncNeoTrellisLEDs();
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
