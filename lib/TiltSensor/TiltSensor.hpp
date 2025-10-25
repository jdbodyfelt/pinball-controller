#ifndef TILTSENSOR_HPP
#define TILTSENSOR_HPP

#include <math.h>
#include <Wire.h>

enum class TiltEvent {
    NONE = 0,
    BUMP,           // Quick nudge/impact
    TILT_WARNING,   // Sustained movement warning
    TILT,           // Tilt penalty - ball loss
    SLAM_TILT,      // Hard impact - immediate ball loss
    FREE_FALL,      // Machine lifted off surface!
    STABLE          // No significant movement
};

class TiltSensor {
private:
    // Byte size for reading
    static const uint8_t READ_ONE_BYTE = 1;
    static const uint8_t READ_SIX_BYTES = 6;

    // ADXL345 Register Addresses
    static const uint8_t ADXL345_ADDR = 0x53;       // I2C address for ADXL345
    static const uint8_t REG_DEVID = 0x00;          // Device ID register
    static const uint8_t REG_POWER_CTL = 0x2D;      // Power control register
    static const uint8_t REG_DATA_FORMAT = 0x31;    // Data format register
    static const uint8_t REG_DATAX0 = 0x32;         // X-Axis Data 0 register
    uint8_t REG_BW_RATE = 0x2C;                     // Data rate register
    static const uint8_t REG_INT_SOURCE = 0x30;     // Interrupt source register
  
    // ADXL345 data rates (from datasheet)
    static const uint8_t RATE_3200_HZ = 0x0F;
    static const uint8_t RATE_1600_HZ = 0x0E;
    static const uint8_t RATE_800_HZ  = 0x0D;
    static const uint8_t RATE_400_HZ  = 0x0C;
    static const uint8_t RATE_200_HZ  = 0x0B;
    static const uint8_t RATE_100_HZ  = 0x0A;
    static const uint8_t RATE_50_HZ   = 0x09;
    static const uint8_t RATE_25_HZ   = 0x08;
    static const uint8_t RATE_12_5_HZ = 0x07;
    static const uint8_t RATE_6_25_HZ = 0x06;
    static const uint8_t RATE_3_13_HZ = 0x05;
    static const uint8_t RATE_1_56_HZ = 0x04;
    static const uint8_t RATE_0_78_HZ = 0x03;
    static const uint8_t RATE_0_39_HZ = 0x02;
    static const uint8_t RATE_0_20_HZ = 0x01;
    static const uint8_t RATE_0_10_HZ = 0x00;

    // Pins for I2C comms & interrupts
    uint8_t sdaPin, sclPin, int1Pin, int2Pin;

    // Sampling rate control & tilt state
    unsigned long sampleInterval;   // in milliseconds
    unsigned long lastSampleTime;   // timestamp of last sample
    unsigned long tiltStartTime;   // timestamp when tilt started
    bool tiltWarningActive;

    // Physical thresholds in g-forces
    float bumpThreshold = 5.0;        // for bump detection
    float tiltAccelThreshold = 2.0;   // for tilt detection
    float slamTiltThreshold = 15.0;   // for slam tilt detection
    float freeFallThreshold = 0.5;    // for free fall detection
    unsigned long tiltTimeThreshold = 2000; // in milliseconds

    // Interrupt flags
    volatile bool int1Triggered;
    volatile bool int2Triggered;

    // Sampling functions
    bool setDataRate(uint8_t rateCode); 
    uint8_t frequencyToRateCode(float frequency);

    // I2C functions
    bool writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
    void readAndPrintInterruptSource();

    // Interrupt handlers (static to be used as Interrupt Service Routine)
    static void handleINT1();
    static void handleINT2();

    // Static pointers for ISR access to instance
    static TiltSensor* instance; 

    // Event detection
    TiltEvent detectEvent(float x, float y, float z);
    void printEvent(TiltEvent event);

public:
    TiltSensor(uint8_t sda = 8, uint8_t scl = 9, uint8_t int1 = 1, uint8_t int2 = 2);
    bool begin(float samplingFrequency = 10.0);     // Default 10Hz
    TiltEvent update();
    void setSamplingRate(float frequency);      // Change rate dynamically!
    void readAcceleration(float &x, float &y, float &z);
    float getCurrentSamplingRate();

    // Configuration setters
    void setBumpThreshold(float threshold) { bumpThreshold = threshold; }
    void setTiltThreshold(float threshold) { tiltAccelThreshold = threshold; }
    void setSlamTiltThreshold(float threshold) { slamTiltThreshold = threshold; }
    void setFreeFallThreshold(float threshold) { freeFallThreshold = threshold; }
    void setTiltTimeThreshold(unsigned long threshold) { tiltTimeThreshold = threshold; }
};

#endif // TILTSENSOR_HPP