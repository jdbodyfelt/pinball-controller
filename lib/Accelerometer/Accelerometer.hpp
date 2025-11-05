#ifndef ACCELEROMETER_HPP
#define ACCELEROMETER_HPP

#pragma once

#include <Adafruit_ADXL345_U.h>

#include "vectors.hpp"
#include "utilities.hpp"

constexpr uint32_t DEVICE_IDENTIFER = 78810; 

/*************************************************************************************/
class Accelerometer {

private:

    Adafruit_ADXL345_Unified sensor;
    range_t range;      // Set on constructor
    dataRate_t rate;
           
public:

    bool _init = false; 
    float _tic; 
    Vec3f coords; 

    Accelerometer(                                   // Simplified constructor
        int32_t sensor_id = DEVICE_IDENTIFER,
        range_t sensor_range = ADXL345_RANGE_8_G, 
        dataRate_t sensor_rate = ADXL345_DATARATE_800_HZ
    );   
    
    bool begin();
    bool readRaw(sensors_event_t* event);
    bool read(); 
    float getDataFreq(); 
    

};
/*************************************************************************************/

#endif