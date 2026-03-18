/*
 * @Description: Bluetooth BLE-MIDI control
 * @Author: kosamit
 */

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "../globals.h"

void startBluetooth();
void stopBluetooth();
void toggleBluetooth();
void onBLEConnected();
void onBLEDisconnected();

#endif // BLUETOOTH_H
