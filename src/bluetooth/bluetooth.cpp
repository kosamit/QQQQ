/*
 * @Description: Bluetooth BLE-MIDI control
 * @Author: kosamit
 */

#include "bluetooth.h"
#include "../screens/screens.h"
#include <esp_bt.h>
#include <esp_bt_main.h>

// Bluetooth開始
void startBluetooth() {
    if (!bleAdvertising) {
        Serial.println("========================================");
        Serial.println("Starting BLE-MIDI...");

        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE) {
            esp_bt_controller_init(&bt_cfg);
            esp_bt_controller_enable(ESP_BT_MODE_BLE);
            esp_bluedroid_init();
            esp_bluedroid_enable();
            Serial.println("BLE hardware initialized");
        }

        delay(100);

        BLEMIDI.setHandleConnected(onBLEConnected);
        BLEMIDI.setHandleDisconnected(onBLEDisconnected);

        MIDI.begin(MIDI_CHANNEL_OMNI);

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

        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();

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
        Serial.println("Preparing for reconnection...");
        delay(1000);

        BLEMIDI.setHandleConnected(onBLEConnected);
        BLEMIDI.setHandleDisconnected(onBLEDisconnected);
        MIDI.begin(MIDI_CHANNEL_OMNI);

        Serial.println("========================================");
        Serial.println("Ready for reconnection");
        Serial.println("Status: Advertising...");
        Serial.println("========================================");
    }

    if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        drawBluetoothStatus();
        xSemaphoreGive(displayMutex);
    }
}
