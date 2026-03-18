/*
 * @Description: Common Functions for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#ifndef COMMON_FUNCTIONS_H
#define COMMON_FUNCTIONS_H

#include "common_config.h"

// Common Display Functions
void GFX_Print_TEST(String s);
void GFX_Print_1();
void GFX_Print_2();
void GFX_Print_FINISH();
void GFX_Print_START();
void GFX_Print_Time_Info_Loop();

// Common Utility Functions
void Skip_Test_Loop();
void Vibration_Start(void);

#endif // COMMON_FUNCTIONS_H
