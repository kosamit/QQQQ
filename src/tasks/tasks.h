/*
 * @Description: FreeRTOS task definitions
 * @Author: kosamit
 */

#ifndef TASKS_H
#define TASKS_H

#include "../globals.h"

// FreeRTOS タスク関数
void touchTask(void* parameter);
void displayTask(void* parameter);
void clockTask(void* parameter);

#endif // TASKS_H
