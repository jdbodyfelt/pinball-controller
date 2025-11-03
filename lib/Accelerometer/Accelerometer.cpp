#include "Accelerometer.hpp"

/*************************************************************************************/
// Simplified constructor
Accelerometer::Accelerometer(int32_t sensor_id, uint8_t sda_pin, uint8_t scl_pin,
    range_t sensor_range, dataRate_t sensor_rate
) : 
    sensor(sensor_id), _sda(sda_pin), _scl(scl_pin), 
    range(sensor_range), rate(sensor_rate)
{}
    
/*************************************************************************************/
bool Accelerometer::begin() {
    // Initialize sensor
    Wire.begin(_sda, _scl);             // Initialize I2C (Global Wire)
    Wire.setClock(400000);              // Set I2C comm channel (vs. 100Hz standard)
    delay(100);                         // Let I2C stabilize
        
    if (!sensor.begin()) {              // Initialize sensor
        Serial.println("ADXL345 not found!");
        return false;
    }
    sensor.setRange(range);             // Configure sensor
    sensor.setDataRate(rate);
    _init = true;
    Serial.println("ADXL345 initialized successfully");
    return calibrate();
}
/*************************************************************************************/        
bool Accelerometer::readRaw(sensors_event_t* event){
    if(!_init){ return false; }
    sensor.getEvent(event);
    coords = Vec3f({
        event->acceleration.x, 
        event->acceleration.y, 
        event->acceleration.z});
    tilt = TiltAngles(coords); 
    return true;
}
/*************************************************************************************/        
bool Accelerometer::readCalibrated(sensors_event_t* event){
    if(!readRaw(event)){ return false; }
    tilt -= zeroTilt; 
    return true;
}
/*************************************************************************************/        
bool Accelerometer::readFiltered(sensors_event_t* event){
    if(!readCalibrated(event)){ return false; }

    /* CONTINUE HERE - ADD get() function to wrap around all */
    
    return true;
}
/*************************************************************************************/        
bool Accelerometer::get(sensors_event_t* event){
    if(!readFiltered(event)){ return false; }

    /* CONTINUE HERE - ADD get() function to wrap around all */
    
    return true;
}


/*************************************************************************************/     
bool Accelerometer::calibrate(uint16_t count) {
    uint16_t current = 0; 
    sensors_event_t event; 
    while (current < count){
        if(! readRaw(&event) ) { return false; };
        zeroTilt += tilt; 
        current += 1; 
        delay(50); 
    }
    zeroTilt /= current; 
    printf("Offset Calibration: Pitch = %.3f°, Roll = %.3f°\n", zeroTilt.pitch, zeroTilt.roll);
    return true; 
}
/*************************************************************************************/