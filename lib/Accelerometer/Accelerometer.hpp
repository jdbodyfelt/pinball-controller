#ifndef ACCELEROMETER_HPP
#define ACCELEROMETER_HPP

#pragma once

#include <Wire.h>
#include "constants.hpp"
#include "vectors.hpp"


/***************************************************************************/


inline float now(){ return 1e-6 * micros(); }

/****************************************************************************/
class Accelerometer 
{
private:
    static Accelerometer* instance;  // Static pointers for ISR access to instance

    float dt;                        // Sampling frequency, assigned in setRateParams
    uint32_t dtu;                    // Same as dt, but in microsecond integer count
    float timeout;                   // Timeout for sensor reading (setRangeParams)
    uint32_t _delay;                 // Reading delay (dt / 10 effectively)
    uint8_t rate;                    // Sampling rate code, defined @ instance
    uint8_t range;                   // Sensor store of a constants.RANGE_<X>G

    uint8_t addr = I2C_ADDRESS_LO;   // Set the I2C HW Address (ADXL345 options)
    uint8_t sda = 8, scl = 9;        // Set the I2C Data & Clock Bus Pins    

    float lpf;                       // Lowpass Cutoff Frequency 
    float bwcQ;                      // Butterworth Characteristic Q
    vec3bw filts_;                   // Butterworth Filter Bank 

    float tiltAngle;                 // Maximum Tilt Angle - constrain to [10,20]

    /********* Internal Methods **********/

    // ADXL345 Registry Accessors     
    uint8_t readRegister(uint8_t reg);        
    bool writeRegister(uint8_t reg, uint8_t val); 

    // Setup Functions
    bool checkDevice();
    bool setRateParams();
    
    // Measurements
    Vec3f read();
    
public:

    /********* External Methods **********/
    // Constructors
    Accelerometer(
        uint8_t dataRange = RANGE_2G, 
        uint8_t dataRate = RATE_100HZ,
        float cutoffFreq = 5.0f,
        float qFactor = 0.707,
        float maxTiltAngle = 12.5f
    );
    bool begin(); 
 
    xyCoords toJoystick();          // Normalised Joystick Output!

};
/*************************************************************************** */
#endif // ACCELEROMETER_HPP