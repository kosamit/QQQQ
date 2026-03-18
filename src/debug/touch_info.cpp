/*
 * @Description: Touch Info Module Implementation for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "touch_info.h"

// Global Touch Info Variable Definition
TouchInfo global_touch_info;

void Init_Touch_Info()
{
    // Initialize all touch coordinates to 0
    for (int i = 0; i < 5; i++) {
        global_touch_info.touch_x[i] = 0;
        global_touch_info.touch_y[i] = 0;
        global_touch_info.prev_touch_x[i] = 0;
        global_touch_info.prev_touch_y[i] = 0;
    }
    global_touch_info.fingers_number = 0;
    global_touch_info.is_touched = false;
    global_touch_info.prev_fingers_number = 0;
    global_touch_info.prev_is_touched = false;
    global_touch_info.has_changed = false;
}

void Update_Touch_Info()
{
    // Store previous values
    for (int i = 0; i < 5; i++) {
        global_touch_info.prev_touch_x[i] = global_touch_info.touch_x[i];
        global_touch_info.prev_touch_y[i] = global_touch_info.touch_y[i];
    }
    global_touch_info.prev_fingers_number = global_touch_info.fingers_number;
    global_touch_info.prev_is_touched = global_touch_info.is_touched;
    
    // Read new touch information from CST226SE
    global_touch_info.touch_x[0] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH1_COORDINATE_X);
    global_touch_info.touch_y[0] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH1_COORDINATE_Y);
    global_touch_info.touch_x[1] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH2_COORDINATE_X);
    global_touch_info.touch_y[1] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH2_COORDINATE_Y);
    global_touch_info.touch_x[2] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH3_COORDINATE_X);
    global_touch_info.touch_y[2] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH3_COORDINATE_Y);
    global_touch_info.touch_x[3] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH4_COORDINATE_X);
    global_touch_info.touch_y[3] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH4_COORDINATE_Y);
    global_touch_info.touch_x[4] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH5_COORDINATE_X);
    global_touch_info.touch_y[4] = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH5_COORDINATE_Y);
    global_touch_info.fingers_number = CST226SE->IIC_Read_Device_Value(CST226SE->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);
    
    // Rotate coordinates for rotation=3 (270 degrees clockwise / 90 degrees counter-clockwise)
    // Original screen: 222x480, Rotated screen: 480x222
    // Transform: new_x = LCD_HEIGHT - 1 - old_y, new_y = old_x
    for (int i = 0; i < 5; i++) {
        int32_t old_x = global_touch_info.touch_x[i];
        int32_t old_y = global_touch_info.touch_y[i];
        global_touch_info.touch_x[i] = LCD_HEIGHT - 1 - old_y;
        global_touch_info.touch_y[i] = old_x;
    }
    
    // Debug output to check raw values (コメントアウトで高速化)
    // if (global_touch_info.fingers_number > 0) {
    //     Serial.printf("Rotated: Fingers=%d ", global_touch_info.fingers_number);
    //     for (int i = 0; i < global_touch_info.fingers_number && i < 5; i++) {
    //         Serial.printf("T%d:(%d,%d) ", i+1, 
    //                       global_touch_info.touch_x[i], global_touch_info.touch_y[i]);
    //     }
    //     Serial.println();
    // }
    
    // Filter out invalid coordinates
    // 座標が0以下、または回転後の画面サイズ(480x222)を超える場合は無効とする
    for (int i = 0; i < 5; i++) {
        if (global_touch_info.touch_x[i] <= 0 || global_touch_info.touch_y[i] <= 0 ||
            global_touch_info.touch_x[i] > LCD_HEIGHT || global_touch_info.touch_y[i] > LCD_WIDTH) {
            global_touch_info.touch_x[i] = 0;
            global_touch_info.touch_y[i] = 0;
        }
    }
    
    // Set touched flag based on finger number and valid coordinates
    global_touch_info.is_touched = (global_touch_info.fingers_number > 0);
    
    // Check for changes
    global_touch_info.has_changed = false;
    
    // Check if finger number changed
    if (global_touch_info.fingers_number != global_touch_info.prev_fingers_number) {
        global_touch_info.has_changed = true;
    }
    
    // Check if touch state changed
    if (global_touch_info.is_touched != global_touch_info.prev_is_touched) {
        global_touch_info.has_changed = true;
    }
    
    // Check if any coordinate changed (only check valid coordinates)
    for (int i = 0; i < 5; i++) {
        if (global_touch_info.touch_x[i] != global_touch_info.prev_touch_x[i] ||
            global_touch_info.touch_y[i] != global_touch_info.prev_touch_y[i]) {
            global_touch_info.has_changed = true;
            break;
        }
    }
}

void Print_Touch_Info()
{
    gfx->fillRect(0, 40, LCD_WIDTH, LCD_HEIGHT / 2, WHITE);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);

    gfx->setCursor(20, 65);
    gfx->printf("Fingers Number:%d ", global_touch_info.fingers_number);

    // Display only valid coordinates when fingers are detected
    if (global_touch_info.fingers_number > 0) {
        int y_pos = 80;
        for (int i = 0; i < 5; i++) {
            if (global_touch_info.touch_x[i] > 0 && global_touch_info.touch_y[i] > 0) {
                gfx->setCursor(20, y_pos);
                gfx->printf("Touch X%d:%d Y%d:%d ", i+1, global_touch_info.touch_x[i], i+1, global_touch_info.touch_y[i]);
                y_pos += 15;
            }
        }
    }
}
