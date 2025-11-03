#pragma once

#include <Adafruit_ADXL345_U.h>
#include <Wire.h>
#include "vectors.hpp"

constexpr uint32_t DEVICE_IDENTIFER = 78810; 
constexpr uint8_t SDA_PIN = 8; 
constexpr uint8_t SCL_PIN = 9;
/*************************************************************************************/
class TiltAngles {
public:
    float pitch, roll; 
    // Copy Constructor
    TiltAngles()
        : pitch(0.f), roll(0.f) {};
    TiltAngles(const Vec3f& vec) {
        pitch = vec.pitch(); 
        roll = vec.roll(); 
    };
    TiltAngles& operator+=(const TiltAngles& other) {
        pitch += other.pitch;
        roll += other.roll; 
        return *this; 
    }
    // -= Operator
    TiltAngles& operator-=(const TiltAngles& other) {
        pitch -= other.pitch;
        roll -= other.roll; 
        return *this; 
    }
    TiltAngles& operator/=(const uint16_t count) {
        pitch /= static_cast<float>(count);
        roll /= static_cast<float>(count); 
        return *this; 
    }
    void print(Stream &stream){
        stream.printf("%.3f\t%.3f\n", pitch, roll);
    }
};
/*************************************************************************************/
class Accelerometer {

private:

    Adafruit_ADXL345_Unified sensor;
    bool _init = false; 

    uint8_t _sda, _scl;   // Set on constructor
    range_t range;
    dataRate_t rate;
    float maxTilt; 

    Vec3f coords; 
    TiltAngles tilt;                 // Pitch & Roll 
    TiltAngles zeroTilt, prevTilt;          
    const float ALPHA = 0.2;             
    
public:
    Accelerometer(
        int32_t sensor_id = DEVICE_IDENTIFER,
        uint8_t sda_pin = SDA_PIN, uint8_t scl_pin = SCL_PIN,  
        range_t sensor_range = ADXL345_RANGE_8_G, 
        dataRate_t sensor_rate = ADXL345_DATARATE_800_HZ
    );    // Simplified constructor
    
    bool begin();
    bool read(sensors_event_t* event, bool filtered = false);
    bool calibrate(uint16_t count = 128); 
};