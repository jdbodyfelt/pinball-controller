#include "Accelerometer.hpp"

/*************************************************************************************/
// Simplified constructor
Accelerometer::Accelerometer(int32_t sensor_id, range_t sensor_range, dataRate_t sensor_rate) 
    : sensor(sensor_id), range(sensor_range), rate(sensor_rate)
{}
/*************************************************************************************/
bool Accelerometer::begin() {
    setWire();                                  // Define I2C specs
    if (!sensor.begin()) { 
        return ErrorMsg("ADXL345 not found!"); 
    }
    sensor.setRange(range);                     // Configure sensor
    sensor.setDataRate(rate);
    _init = true;
    Serial.println("ADXL345 initialized successfully");
    msgPause();
    return true;                         
}
/*************************************************************************************/        
bool Accelerometer::readRaw(sensors_event_t* event){
    if(!_init){ return ErrorMsg("Run begin()"); }
    if(!sensor.getEvent(event)){ return ErrorMsg("No sensor event!"); }
    _tic = 1e-6 * micros();
    coords = Vec3f({
        event->acceleration.x, 
        event->acceleration.y, 
        event->acceleration.z});
    return true;
}
/*************************************************************************************/          
bool Accelerometer::read(){
    sensors_event_t event; 
    if(!readRaw(&event)){ return false; }
    return true; 
}
/*************************************************************************************/
float Accelerometer::getDataFreq() {
    switch(rate) {
        case ADXL345_DATARATE_3200_HZ: return 3200.0;
        case ADXL345_DATARATE_1600_HZ: return 1600.0;
        case ADXL345_DATARATE_800_HZ:  return 800.0;
        case ADXL345_DATARATE_400_HZ:  return 400.0;
        case ADXL345_DATARATE_200_HZ:  return 200.0;
        case ADXL345_DATARATE_100_HZ:  return 100.0;
        case ADXL345_DATARATE_50_HZ:   return 50.0;
        case ADXL345_DATARATE_25_HZ:   return 25.0;
        case ADXL345_DATARATE_12_5_HZ: return 12.5;
        case ADXL345_DATARATE_6_25HZ:  return 6.25;
        case ADXL345_DATARATE_3_13_HZ: return 3.13;
        case ADXL345_DATARATE_1_56_HZ: return 1.56;
        case ADXL345_DATARATE_0_78_HZ: return 0.78;
        case ADXL345_DATARATE_0_39_HZ: return 0.39;
        case ADXL345_DATARATE_0_20_HZ: return 0.20;
        case ADXL345_DATARATE_0_10_HZ: return 0.10;
        default: return 100.0; // Default to 100 Hz
    }
}
/*************************************************************************************/