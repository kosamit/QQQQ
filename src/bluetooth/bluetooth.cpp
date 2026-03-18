/*
 * @Description: Bluetooth BLE-MIDI control
 * @Author: kosamit
 */

#include "bluetooth.h"
#include "../screens/screens.h"
#include <NimBLEDevice.h>

// Bluetooth開始
void startBluetooth() {
    if (!bleAdvertising) {
        Serial.println("========================================");
        Serial.println("Starting BLE-MIDI...");

        BLEMIDI.setHandleConnected(onBLEConnected);
        BLEMIDI.setHandleDisconnected(onBLEDisconnected);

        MIDI.begin(MIDI_CHANNEL_OMNI);

        // アドバタイジングデータにデバイス名を設定
        NimBLEServer* pServer = NimBLEDevice::getServer();
        if (pServer) {
            NimBLEAdvertising* pAdvertising = pServer->getAdvertising();
            pAdvertising->setName(DEVICE_NAME);
            pAdvertising->start();
        }

        delay(100);

        bleAdvertising = true;

        Serial.println("BLE-MIDI started successfully");
        Serial.println(String("Device name: ") + DEVICE_NAME);
        Serial.println("Status: Advertising...");
        Serial.println("========================================");
    } else {
        Serial.println("BLE already running");
    }
}

// Bluetooth停止
void stopBluetooth() {
    if (bleAdvertising) {
        Serial.println("========================================");
        Serial.println("Stopping BLE-MIDI...");

        bleConnected = false;
        bleAdvertising = false;

        BLEDevice::deinit(false);

        Serial.println("BLE-MIDI stopped completely");
        Serial.println("BLE hardware disabled");
        Serial.println("========================================");
    }
}

// Bluetoothトグル
void toggleBluetooth() {
    if (bleAdvertising) {
        stopBluetooth();
    } else {
        startBluetooth();
    }
}

// BLE接続時のコールバック
void onBLEConnected() {
    Serial.println("========================================");
    Serial.println("BLE CONNECTED!");
    Serial.println("Device is now connected");
    Serial.println("========================================");

    bleConnected = true;

    if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        drawBluetoothStatus();
        xSemaphoreGive(displayMutex);
    }
}

// BLE切断時のコールバック
void onBLEDisconnected() {
    Serial.println("========================================");
    Serial.println("BLE DISCONNECTED");
    Serial.println("========================================");

    bleConnected = false;

    if (bleAdvertising) {
        Serial.println("Status: Advertising (waiting for reconnection)...");
    }

    if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        drawBluetoothStatus();
        xSemaphoreGive(displayMutex);
    }
}
