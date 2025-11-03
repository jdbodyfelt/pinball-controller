#include <Arduino.h>
#include "debug.hpp"
/*
This holds various constants for the ADXL345 Accelerometer. 
Uses a self-written Wire version for full control. 
Also uses FIFO buffer for finite difference stores.
*/
#pragma once

// Byte & buffer sizes for reading
constexpr uint8_t READ_ONE_BYTE = 1;
constexpr uint8_t READ_SIX_BYTES = 6;

// I2C Address Possibles
constexpr uint8_t I2C_ADDRESS_LO = 0x53;
constexpr uint8_t I2C_ADDRESS_HI = 0x1D;

// Sensor Register Definitions
constexpr uint8_t REG_DEVID = 0x00;          // Device ID - should return 0xE5!
constexpr uint8_t REG_BW_RATE = 0x2C;        // Data rate and power mode control
constexpr uint8_t REG_POWER_CTL = 0x2D;      // Power-saving features control
constexpr uint8_t REG_DATA_FORMAT = 0x31;    // Controls data format and measurement range
constexpr uint8_t REG_INT_ENABLE = 0x2E;     // Enables specific interrupt sources.
constexpr uint8_t REG_INT_SOURCE = 0x30;     // Tells which interrupts actually fire.
constexpr uint8_t REG_FIFO_CTL = 0x38;
constexpr uint8_t REG_FIFO_STATUS = 0x39; 
constexpr uint8_t REG_DATAX0 = 0x32;

// Power control values
constexpr uint8_t POWER_MEASURE = 0x08;  // Measurement mode
constexpr uint8_t POWER_STANDBY = 0x00;  // Standby mode

// DATA_FORMAT Register (0x31) constants
constexpr uint8_t FORMAT_SELF_TEST   = 0x80;  // Self-test force
constexpr uint8_t FORMAT_SPI         = 0x40;  // 3-wire SPI mode when set
constexpr uint8_t FORMAT_INT_INVERT  = 0x20;  // Interrupt active high when set
constexpr uint8_t FORMAT_FULL_RES    = 0x08;  // Full resolution mode
constexpr uint8_t FORMAT_JUSTIFY     = 0x04;  // Left justify when set (0=right justify)

// Sensor Ranges (bits 1-0)
constexpr uint8_t RANGE_2G  = 0x00;  // ±2g 
constexpr uint8_t RANGE_4G  = 0x01;  // ±4g
constexpr uint8_t RANGE_8G  = 0x02;  // ±8g
constexpr uint8_t RANGE_16G = 0x03;  // ±16g

// Sensor Data Rates
constexpr uint8_t RATE_3200HZ = 0x0F;
constexpr uint8_t RATE_1600HZ = 0x0E;
constexpr uint8_t RATE_800HZ  = 0x0D;
constexpr uint8_t RATE_400HZ  = 0x0C;
constexpr uint8_t RATE_200HZ  = 0x0B;
constexpr uint8_t RATE_100HZ  = 0x0A;
constexpr uint8_t RATE_50HZ   = 0x09;
constexpr uint8_t RATE_25HZ   = 0x08;
constexpr uint8_t RATE_12_5HZ = 0x07;
constexpr uint8_t RATE_6_25HZ = 0x06;
constexpr uint8_t RATE_3_13HZ = 0x05;
constexpr uint8_t RATE_1_56HZ = 0x04;
constexpr uint8_t RATE_0_78HZ = 0x03;
constexpr uint8_t RATE_0_39HZ = 0x02;
constexpr uint8_t RATE_0_20HZ = 0x01;
constexpr uint8_t RATE_0_10HZ = 0x00;

// Quick Binary-to-Float Frequency Converter
inline float getFrequency(uint8_t rateCode) {
    static const float freqs [] = {
        0.10f,   // 0x00: RATE_0_10HZ
        0.20f,   // 0x01: RATE_0_20HZ  
        0.39f,   // 0x02: RATE_0_39HZ
        0.78f,   // 0x03: RATE_0_78HZ
        1.56f,   // 0x04: RATE_1_56HZ
        3.13f,   // 0x05: RATE_3_13HZ
        6.25f,   // 0x06: RATE_6_25HZ
        12.5f,   // 0x07: RATE_12_5HZ
        25.0f,   // 0x08: RATE_25HZ
        50.0f,   // 0x09: RATE_50HZ
        100.0f,  // 0x0A: RATE_100HZ
        200.0f,  // 0x0B: RATE_200HZ
        400.0f,  // 0x0C: RATE_400HZ
        800.0f,  // 0x0D: RATE_800HZ
        1600.0f, // 0x0E: RATE_1600HZ
        3200.0f  // 0x0F: RATE_3200HZ
    };
    // Simple bounds check and array lookup
    if (rateCode <= 0x0F) {
        return freqs[rateCode];
    }
    return 0.0f; // Default for invalid values
}



// FIFO Control Register Modes (REG_FIFO_CTL bits 7-6)
constexpr uint8_t FIFO_MODE_BYPASS   = 0x00;  // Bypass FIFO
constexpr uint8_t FIFO_MODE_FIFO     = 0x40;  // FIFO mode (stops when full)
constexpr uint8_t FIFO_MODE_STREAM   = 0x80;  // Stream mode (continuous)
constexpr uint8_t FIFO_MODE_TRIGGER  = 0xC0;  // Trigger mode
constexpr uint8_t FIFO_MODE_MASK     = 0xC0;  // Mode selection mask

// FIFO Control Register Sample Thresholds (REG_FIFO_CTL bits 4-0)
constexpr uint8_t FIFO_SAMPLES_MASK  = 0x1F;  // Sample threshold mask
constexpr uint8_t FIFO_SAMPLES_1     = 0x00;  // 1 sample  (0+1)
constexpr uint8_t FIFO_SAMPLES_2     = 0x01;  // 2 samples (1+1)
constexpr uint8_t FIFO_SAMPLES_4     = 0x03;  // 4 samples (3+1)
constexpr uint8_t FIFO_SAMPLES_8     = 0x07;  // 8 samples (7+1)
constexpr uint8_t FIFO_SAMPLES_16    = 0x0F;  // 16 samples (15+1)
constexpr uint8_t FIFO_SAMPLES_32    = 0x1F;  // 32 samples (31+1)
// Note: FIFO_SAMPLES_64 = 0x3F is invalid (exceeds 5-bit mask)

// FIFO Status Register (REG_FIFO_STATUS)
constexpr uint8_t FIFO_STATUS_TRIGGER_BIT = 0x80;    // Bit 7: Trigger occurred
constexpr uint8_t FIFO_STATUS_COUNT_MASK  = 0x3F;    // Bits 5-0: Samples in FIFO

// Uncomment if needed for trigger configuration:
// constexpr uint8_t FIFO_TRIGGER_INT2 = 0x20;  // Trigger on INT2 (0 = INT1)

// Interrupt Enable Options
constexpr uint8_t INT_DATA_READY  = 0x80; // New data available
constexpr uint8_t INT_SINGLE_TAP  = 0x40; // Single tap detected  
constexpr uint8_t INT_DOUBLE_TAP  = 0x20; // Double tap detected
constexpr uint8_t INT_ACTIVITY    = 0x10; // Acceleration above threshold
constexpr uint8_t INT_INACTIVITY  = 0x08; // Acceleration below threshold  
constexpr uint8_t INT_FREE_FALL   = 0x04; // Free-fall condition
constexpr uint8_t INT_WATERMARK   = 0x02; // FIFO watermark reached
constexpr uint8_t INT_OVERRUN     = 0x01; // FIFO overrun