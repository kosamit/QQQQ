/*
 * @Description: I2C Scanner - scan both Wire and Wire1
 */
#include <Arduino.h>
#include <Wire.h>
#include "pin_config.h"

#define SDA_PIN IIC_SDA
#define SCL_PIN IIC_SCL

void scan_i2c_device(TwoWire &i2c, const char* name)
{
    Serial.printf("\n=== %s ===\n", name);
    Serial.println("Scanning for I2C devices ...");
    Serial.print("      ");
    for (int i = 0; i < 0x10; i++)
    {
        Serial.printf("0x%02X|", i);
    }
    uint8_t error;
    for (int j = 0; j < 0x80; j += 0x10)
    {
        Serial.println();
        Serial.printf("0x%02X |", j);
        for (int i = 0; i < 0x10; i++)
        {
            i2c.beginTransmission(i | j);
            error = i2c.endTransmission();
            if (error == 0)
                Serial.printf("0x%02X|", i | j);
            else
                Serial.print(" -- |");
        }
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo - I2C Scanner");

    pinMode(RT9080_EN, OUTPUT);
    digitalWrite(RT9080_EN, HIGH);

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire1.begin(43, 44);
}

void loop()
{
    scan_i2c_device(Wire, "Wire (SDA=5, SCL=6)");
    scan_i2c_device(Wire1, "Wire1 (SDA=43, SCL=44)");
    Serial.println("\n--- Waiting 3 seconds ---\n");
    delay(3000);
}
