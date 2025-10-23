/*
 * @Description: Touch Info Module Implementation for Qurospad
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
    
    // Debug output to check raw values (uncomment if needed for debugging)
    // Serial.printf("Raw values - Fingers: %d, X1: %d, Y1: %d, X2: %d, Y2: %d\n", 
    //               global_touch_info.fingers_number,
    //               global_touch_info.touch_x[0], global_touch_info.touch_y[0],
    //               global_touch_info.touch_x[1], global_touch_info.touch_y[1]);
    
    // Filter out invalid coordinates (8 seems to be a default/invalid value)
    for (int i = 0; i < 5; i++) {
        if (global_touch_info.touch_x[i] == 8 || global_touch_info.touch_y[i] == 8 ||
            global_touch_info.touch_x[i] <= 0 || global_touch_info.touch_y[i] <= 0) {
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
