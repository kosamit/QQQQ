/*
 * @Description: Music Test Module for Qurospad
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#ifndef MUSIC_TEST_H
#define MUSIC_TEST_H

#include "common_config.h"

// Music Test Functions
void GFX_Print_Play_Progress(void);
void GFX_Print_Volume();
void GFX_Print_Music_Volume_Value();
void GFX_Print_Play_Failed();
void Music_Test_Display();
bool Music_Test_Loop();

#endif // MUSIC_TEST_H
