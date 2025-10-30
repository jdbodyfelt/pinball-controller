#include <Arduino.h>
/*
This holds various constants for the ADXL345 Accelerometer. 
Uses a self-written Wire version for full control. 
Also uses FIFO buffer for finite difference stores.
*/
#pragma once

// Byte & buffer sizes for reading
static const uint8_t READ_ONE_BYTE = 1;
static const uint8_t READ_SIX_BYTES = 6;

// I2C Address Possibles
static const uint8_t I2C_ADDRESS_LO = 0x53;
static const uint8_t I2C_ADDRESS_HI = 0x1D;

// Sensor Register Definitions
static const uint8_t REG_DEVID = 0x00;          // Device ID - should return 0xE5!
static const uint8_t REG_BW_RATE = 0x2C;        // Data rate and power mode control
static const uint8_t REG_POWER_CTL = 0x2D;      // Power-saving features control
static const uint8_t REG_DATA_FORMAT = 0x31;    // Controls data format and measurement range
static const uint8_t REG_INT_ENABLE = 0x2E;     // Enables specific interrupt sources.
static const uint8_t REG_INT_SOURCE = 0x30;     // Tells which interrupts actually fire.
static const uint8_t REG_FIFO_CTL = 0x38;
static const uint8_t REG_FIFO_STATUS = 0x39; 
static const uint8_t REG_DATAX0 = 0x32;

// Power control values
static const uint8_t POWER_MEASURE = 0x08;  // Measurement mode
static const uint8_t POWER_STANDBY = 0x00;  // Standby mode

// DATA_FORMAT Register (0x31) constants
static const uint8_t FORMAT_SELF_TEST   = 0x80;  // Self-test force
static const uint8_t FORMAT_SPI         = 0x40;  // 3-wire SPI mode when set
static const uint8_t FORMAT_INT_INVERT  = 0x20;  // Interrupt active high when set
static const uint8_t FORMAT_FULL_RES    = 0x08;  // Full resolution mode
static const uint8_t FORMAT_JUSTIFY     = 0x04;  // Left justify when set (0=right justify)

// Sensor Ranges (bits 1-0)
static const uint8_t RANGE_2G  = 0x00;  // ±2g 
static const uint8_t RANGE_4G  = 0x01;  // ±4g
static const uint8_t RANGE_8G  = 0x02;  // ±8g
static const uint8_t RANGE_16G = 0x03;  // ±16g

// Sensor Data Rates
static const uint8_t RATE_3200HZ = 0x0F;
static const uint8_t RATE_1600HZ = 0x0E;
static const uint8_t RATE_800HZ  = 0x0D;
static const uint8_t RATE_400HZ  = 0x0C;
static const uint8_t RATE_200HZ  = 0x0B;
static const uint8_t RATE_100HZ  = 0x0A;
static const uint8_t RATE_50HZ   = 0x09;
static const uint8_t RATE_25HZ   = 0x08;
static const uint8_t RATE_12_5HZ = 0x07;
static const uint8_t RATE_6_25HZ = 0x06;
static const uint8_t RATE_3_13HZ = 0x05;
static const uint8_t RATE_1_56HZ = 0x04;
static const uint8_t RATE_0_78HZ = 0x03;
static const uint8_t RATE_0_39HZ = 0x02;
static const uint8_t RATE_0_20HZ = 0x01;
static const uint8_t RATE_0_10HZ = 0x00;

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

// FIFO Register Control bits & Sample watermarks
static const uint8_t FIFO_MODE_BYPASS   = 0x00;  // Bypass FIFO
static const uint8_t FIFO_MODE_FIFO     = 0x40;  // FIFO mode (stops when full)
static const uint8_t FIFO_MODE_STREAM   = 0x80;  // Stream mode (continuous)
static const uint8_t FIFO_MODE_TRIGGER  = 0xC0;  // Trigger mode
//static const uint8_t FIFO_TRIG_INT2     = 0x20;  // Trigger on INT2 (0 = INT1)
static const uint8_t FIFO_MASK = 0x3F;      // Mask only sample bits

// FIFO Sample Watermarks (0-31 samples, value = samples-1)
static const uint8_t FIFO_SAMPLES_1   = 0x00;   // 1 sample
static const uint8_t FIFO_SAMPLES_2   = 0x01;   // 2 samples
static const uint8_t FIFO_SAMPLES_4   = 0x03;   // 4 samples
static const uint8_t FIFO_SAMPLES_8   = 0x07;   // 8 samples
static const uint8_t FIFO_SAMPLES_16  = 0x0F;   // 16 samples
static const uint8_t FIFO_SAMPLES_32  = 0x1F;   // 32 samples
static const uint8_t FIFO_SAMPLES_64  = 0x3F;   // 64 samples

// Interrupt Enable Options
static const uint8_t INT_DATA_READY  = 0x80; // New data available
static const uint8_t INT_SINGLE_TAP  = 0x40; // Single tap detected  
static const uint8_t INT_DOUBLE_TAP  = 0x20; // Double tap detected
static const uint8_t INT_ACTIVITY    = 0x10; // Acceleration above threshold
static const uint8_t INT_INACTIVITY  = 0x08; // Acceleration below threshold  
static const uint8_t INT_FREE_FALL   = 0x04; // Free-fall condition
static const uint8_t INT_WATERMARK   = 0x02; // FIFO watermark reached
static const uint8_t INT_OVERRUN     = 0x01; // FIFO overrun
