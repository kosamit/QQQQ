/*
 * @Description: Touch Info Module for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#ifndef TOUCH_INFO_H
#define TOUCH_INFO_H

#include "../common/common_config.h"

// Touch Info Structure
struct TouchInfo {
    int32_t touch_x[5];
    int32_t touch_y[5];
    uint8_t fingers_number;
    bool is_touched;
    
    // Previous values for change detection
    int32_t prev_touch_x[5];
    int32_t prev_touch_y[5];
    uint8_t prev_fingers_number;
    bool prev_is_touched;
    bool has_changed;
};

// Global Touch Info Variable
extern TouchInfo global_touch_info;

// Touch Info Functions
void Print_Touch_Info();
void Update_Touch_Info();
void Init_Touch_Info();

#endif // TOUCH_INFO_H
