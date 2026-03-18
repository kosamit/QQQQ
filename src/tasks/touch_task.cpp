/*
 * @Description: Touch task implementation
 * @Author: kosamit
 */

#include "touch_task.h"
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
