#ifndef ACCELEROMETER_HPP
#define ACCELEROMETER_HPP

#pragma once

#include <Wire.h>
#include <etl/vector.h>
#include <memory>
#include "constants.hpp"
#include "ButterworthFilter.hpp"


/***************************************************************************/
using vec3f = etl::vector<float, 3>;
using vec3bw = etl::vector<ButterworthFilter, 3>;

// Quick Addition Operator for vec3f
inline vec3f add(const vec3f &A, const vec3f &B){
    vec3f result; 
    for (uint8_t k = 0; k < A.size(); k++){
        result[k] = A[k] + B[k]; 
    }
    return result;  
}

// Quick Binary-to-Float Frequency Converter
inline float getFrequency(uint8_t dataRate) {
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
    if (dataRate <= 0x0F) {
        return freqs[dataRate];
    }
    return 0.0f; // Default for invalid values
}

/****************************************************************************/
class Accelerometer 
{
private:
    static Accelerometer* instance;  // Static pointers for ISR access to instance

    float dt;                        // Sampling frequency, assigned in setRateParams
    uint32_t dtu;                    // Same as dt, but in microsecond integer count

    uint8_t rate;                    // Sampling rate code, defined @ instance
    uint8_t range;                   // Sensor store of a constants.RANGE_<X>G

    uint8_t addr = I2C_ADDRESS_LO;   // Set the I2C HW Address (ADXL345 options)
    uint8_t sda = 8, scl = 9;        // Set the I2C Data & Clock Bus Pins    

    vec3f calibration = {0,0,0};     // Mean Calibration Store
    float lpf;                       // Lowpass Cutoff Frequency 
    float bwcQ;                      // Butterworth Characteristic Q
    vec3bw filters;                  // Butterworth Filter Bank 

    float tiltAngle;                 // Maximum Tilt Angle - constrain to [10,20]

    /********* Internal Methods **********/

    // ADXL345 Registry Accessors     
    uint8_t readRegister(uint8_t reg);        
    bool writeRegister(uint8_t reg, uint8_t val); 

    // Setup Functions
    bool checkDevice();
    bool setRateParams();
    
    // Measurements
    vec3f average();
    std::unique_ptr<vec3f> read(bool doFilter = false);
    
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
    bool update(bool doCalibrate);

};
/*************************************************************************** */
#endif // ACCELEROMETER_HPP