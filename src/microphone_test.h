/*
 * @Description: Microphone Test Module for Qurospad
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#ifndef MICROPHONE_TEST_H
#define MICROPHONE_TEST_H

#include "common_config.h"

// Microphone Test Functions
void GFX_Print_Voice_Speaker_Info_Loop(int16_t left_channel, int16_t right_channel);
void Microphone_Test_Display();
bool Microphone_Test_Loop();

#endif // MICROPHONE_TEST_H
