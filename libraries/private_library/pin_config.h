/*
 * @Description: Pin configuration for LilyGo T4-S3 (2.41" AMOLED, RM690B0)
 * @Board: LilyGo T4-S3  https://www.lilygo.cc/products/t4-s3
 * @License: GPL 3.0
 *
 * Source of truth for pins: Xinyuan-LilyGO/LilyGo-AMOLED-Series (BOARD_AMOLED_241)
 */
#pragma once

// ===================== Display: RM690B0 AMOLED (QSPI) =====================
// Panel native (rotation 0) is portrait 450x600; app runs landscape via rotation.
#define LCD_WIDTH 450
#define LCD_HEIGHT 600

#define LCD_QSPI_CS 11
#define LCD_QSPI_SCK 15
#define LCD_QSPI_D0 14
#define LCD_QSPI_D1 10
#define LCD_QSPI_D2 16
#define LCD_QSPI_D3 12
#define LCD_RST 13
#define LCD_TE 18
#define LCD_EN 9 // PMIC / AMOLED power enable — MUST be HIGH before display init
#define LCD_COL_OFFSET 16 // RM690B0 column start offset (tune on hardware if shifted)

// ===================== Touch: CST226SE (I2C, addr 0x5A) ===================
// Shares the I2C bus with the SY6970 PMU.
#define IIC_SDA 6
#define IIC_SCL 7
#define TOUCH_RST 17
#define TOUCH_INT 8

// ===================== PMU: SY6970 (I2C, addr 0x6A) =======================
#define SY6970_Address 0x6A
#define SY6970_INT 5

// ===================== SD card (SPI) =====================================
#define SD_CS 1
#define SD_MISO 4
#define SD_MOSI 2
#define SD_SCLK 3

// ===================== Onboard button ===================================
#define BOARD_BTN 0

// NOTE: T4-S3 has no speaker (MAX98357A), no MEMS mic (MSM261), no RTC crystal
// wired to a PCF85063, no vibration motor, and no separate LCD backlight.
// Those peripherals from the T-Display-S3-Pro build are intentionally omitted.
