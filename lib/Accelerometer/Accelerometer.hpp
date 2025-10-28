#ifndef ACCELEROMETER_HPP
#define ACCELEROMETER_HPP

#include <Wire.h>
#include "vector.hpp"

/*************************************************************************** */
enum class Event {
    NONE = 0,
    BUMP,           // Quick nudge/impact
    TILT_WARNING,   // Sustained movement warning
    TILT,           // Tilt penalty - ball loss
    SLAM_TILT,      // Hard impact - immediate ball loss
    FREE_FALL,      // Machine lifted off surface!
    STABLE          // No significant movement
};

/*************************************************************************** */
class Accelerometer {
private:

    // Register Addresses
    static const uint8_t I2C_ADDR = 83;         // I2C address for ADXL345
    static const uint8_t DEV_ID = 0;            // Device ID 
    static const uint8_t PWR_CTL = 45;          // Power Control
    static const uint8_t DATA_FMT = 49;         // Data Format 
    static const uint8_t DATA_X0 = 50;          // X-Axis Data 0 on Little Endian
    uint8_t BW_RATE = 44;                       // Data Rate 
    static const uint8_t INT_SRC = 48;          // Interrupt source register
  
    // Pins for I2C comms & interrupts
    uint8_t sdaPin, sclPin, intPins[2];

    // Interrupt flags
    volatile bool intTriggered;
    volatile bool int2Triggered;
    
    // Sampling rate control (ms interval & tictoc)
    uint16_t sampleInterval, lastSampleTime;



    // Tilt state
    uint16_t tiltStartTime;    // tictoc for tilt started
    bool tiltWarningActive;

    // Physical thresholds in g-forces
    float bumpThreshold = 2.0;      // for bump detection
    float tiltThreshold = 4.0;      // for tilt detection
    float slamThreshold = 8.0;     // for slam tilt detection
    float freeThreshold = 9.8;      // for free fall detection
    uint16_t tiltTime = 2000;       // in milliseconds
    uint16_t calibrateTime = 1000;  // in milliseconds


    // Sampling function
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

    // Calibration & event detection
    TiltEvent detectEvent(Vector accel);
    void printEvent(TiltEvent event);

public:
    // Constructor
    TiltSensor(
        uint8_t sda = 8, uint8_t scl = 9, uint8_t int1 = 1, uint8_t int2 = 2, 
        uint16_t sampleIntMs = 20     // Default 50Hz (1000/50 = 20ms)
    );  

    bool setSamplingRate(float frequency);      // Change rate dynamically!
    
    bool begin(float samplingFrequency = 100.0);     // Default 100Hz
    
    void update();
    
    Vector readAcceleration();
};

/*************************************************************************** */
#endif // TILTSENSOR_HPP