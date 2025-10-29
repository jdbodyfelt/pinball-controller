#ifndef ACCELEROMETER_HPP
#define ACCELEROMETER_HPP

#include <Wire.h>
#include <etl/vector.h>
#include "constants.hpp"

/*************************************************************************** */
using vec3f =  etl::vector<float, 3>;

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

/*************************************************************************** */
class Accelerometer 
{
private:

    float _dt;            // Sampling frequency, assigned within setDataRate
    uint32_t _dtu;       // Freq, in usec
    float _tiltAngle;     // Maximum Tilt Angle - constrain to [10,20]

    uint8_t _id, _rate, _range;   // Settings defined at instantiation 
    uint8_t _addr = I2C_ADDRESS_LO; 

    // Set the ESP2 Physical Connects
    uint8_t _sda = 8, _scl = 9;     // I2C Bus Pins

    // Data Filtering - Cutoff Freq & IIR Stores
    float _lpf, _hpf;                        
    vec3f _lpf_in = {0,0,0};
    vec3f _hpf_in = {0,0,0}, _hpf_out = {0,0,0}; 

    // Static pointers for ISR access to instance
    static Accelerometer* instance; 

    /*** Internal Methods ***/
    bool checkDevice();
    bool setDataRate();
    bool writeRegister(uint8_t reg, uint8_t val); 
    uint8_t readRegister(uint8_t reg);
    void process(float time, int16_t x, int16_t y, int16_t z);
    vec3f lowpass(const vec3f &vals);
    vec3f highpass(const vec3f &vals);
    bool calibrate(); 
  
public:

    /*** External Methods ***/
    Accelerometer(
        uint8_t dataRange = RANGE_2G, 
        uint8_t dataRate = RATE_100HZ,
        float lowPassCutoff = 5.0f, 
        float highPassCutoff = 20.0f,
        float maxTiltAngle = 12.5f
    );
    bool begin(); 
    void read();

};
/*************************************************************************** */
#endif // ACCELEROMETER_HPP