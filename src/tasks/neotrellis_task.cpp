/*
 * @Description: NeoTrellis polling task
 * @Author: kosamit
 */

#include "neotrellis_task.h"
#include "../neotrellis/neotrellis_handler.h"
#include "../chord/chord_mode.h"

void neotrellisTask(void* parameter)
{
    Serial.println("NeoTrellisタスク開始");

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 20ms間隔でポーリング

    // 未接続時の再スキャン間隔（2秒 = 100 * 20ms）。ホットプラグ対応＋配線診断用。
    const uint32_t RETRY_TICKS = 100;
    uint32_t retryCounter = 0;

    while (true) {
        if (!neotrellisConnected) {
            // 未接続: 一定間隔で再スキャン/再初期化を試行。
            if (++retryCounter >= RETRY_TICKS) {
                retryCounter = 0;
                initNeoTrellis();
            }
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
            continue;
        }

        if (neotrellisConnected && trellis != nullptr) {
            // ボタンイベント読み取り（コールバックが呼ばれる）
            trellis->read();

            // ディスプレイ更新（displayMutex取得）
            if (currentScreen == SCREEN_DRUMPAD && grid != nullptr) {
                if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                    grid->redrawChangedCells();
                    xSemaphoreGive(displayMutex);
                }
                // NeoTrellis LEDをGrid状態に同期（ドラムパッド時のみ）
                syncNeoTrellisLEDs();
            } else if (currentScreen == SCREEN_CHORD) {
                if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                    chordRedrawDirty();
                    xSemaphoreGive(displayMutex);
                }
            }
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
