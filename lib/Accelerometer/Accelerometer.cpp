#include "Accelerometer.hpp"

/****************************************************************************/
Accelerometer::Accelerometer(
    uint8_t dataRange, uint8_t dataRate, 
    float cutoffFreq, float qFactor, float maxTiltAngle
) : 
    range(dataRange), rate(dataRate),
    lpf(cutoffFreq), bwcQ(qFactor), tiltAngle(maxTiltAngle)
 {}
/****************************************************************************/
uint8_t Accelerometer::readRegister(uint8_t reg) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(addr, READ_ONE_BYTE);
    return Wire.available() ? Wire.read() : 0;
} 
/****************************************************************************/
bool Accelerometer::writeRegister(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    return (Wire.endTransmission() == 0);
}
 /****************************************************************************/
bool Accelerometer::checkDevice()
{
    uint8_t devId = readRegister(REG_DEVID);
    char msg[] = "🚨 ADXL345 CRITICAL!";
    // Check device is registered
    Wire.write(devId);
    if( Wire.endTransmission() != 0 ) {
        Serial.printf("%s Bad Write to Device ID: %02X\n", msg, devId); 
        return false;
    }
    // Check the I2C Bus
    Wire.requestFrom(addr, READ_ONE_BYTE);
    if (!Wire.available()) {
        Serial.printf("%s I2C Read Failure on: %02X\n", msg, addr); 
        return false;
    }
    devId = Wire.read();
    if ( devId != 0xE5 ) {
        Serial.printf("%s Bad Read from Device ID: %02X\n", msg, devId); 
        return false; 
    }
    return true; 
}
/****************************************************************************/
bool Accelerometer::setRateParams() {
    // Set the data rate register
    if ( !writeRegister(REG_BW_RATE, rate) ) {
        char msg[] = "🚨 ADXL345 CRITICAL!";
        Serial.printf("%s Bad rate code: %02X\n", msg, rate);
        return false; 
    }
    // Define intervals from the rate code
    float fs = getFrequency(rate);
    dt = 1.0f / fs;                             // Interval Period
    dtu = static_cast<uint32_t>(1e6*dt);        // Period in microsec
    timeout = 2.5 * dt; 
    _delay = static_cast<uint32_t>(1e6*dt)/1e1;
    // Setup Butterworth filters                     
    for(uint8_t k=0; k<3; k++){
        filts_[k].setLowpass(lpf, bwcQ, fs);
    }
    return true; 
}
/****************************************************************************/
Vec3f Accelerometer::read() {
    // Check if new data is ready
    float tic = now();
    while((readRegister(REG_INT_SOURCE) & INT_DATA_READY) == 0) {
        delayMicroseconds(_delay);
        if(now() - tic > timeout){
            Serial.println("WARNING: Timeout on Sensor Read!");
        }
    }
    // Read 6 bytes
    Wire.beginTransmission(addr);
    Wire.write(REG_DATAX0);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, READ_SIX_BYTES);
    if (Wire.available() == 6) {
        // Read raw data bytes
        uint8_t x0 = Wire.read(), x1 = Wire.read();
        uint8_t y0 = Wire.read(), y1 = Wire.read();
        uint8_t z0 = Wire.read(), z1 = Wire.read();
        // Convert to signed 16-bit integers
        int16_t rawX = (int16_t)((x1 << 8) | x0);
        int16_t rawY = (int16_t)((y1 << 8) | y0);
        int16_t rawZ = (int16_t)((z1 << 8) | z0);
        Vec3f vec(rawX, rawY, rawZ);
        vec /= 256.0f;     // Convert to g's, LSBs / g
        return vec;
    }
    else {
        Serial.println("🚨 FAIL: Wire.read()");
        return Vec3f(); 
    }
}
/****************************************************************************/
bool Accelerometer::begin()
{
    // Initiate Comms & Disable FIFO / Interrupts
    Wire.begin(sda, scl);  
    writeRegister(REG_FIFO_CTL, FIFO_MODE_BYPASS); 
    writeRegister(REG_INT_ENABLE, FIFO_MODE_BYPASS);

    checkDevice();                                  // Verify an ADXL345 device
    writeRegister(REG_POWER_CTL, POWER_STANDBY);    // Set on standby for config
    setRateParams();                                // Sets all rate parameters

     // Set the Data Range (2G default)
    uint8_t options = range | FORMAT_FULL_RES;  
    writeRegister(REG_DATA_FORMAT, options);

    // Enable measuring 
    writeRegister(REG_POWER_CTL, POWER_MEASURE);
    return true;
}
/****************************************************************************/
xyCoords Accelerometer::toJoystick(){
    Vec3f rec = read(); 
    rec.filter(filts_); // Apply a lowpass
    float pitch = constrain(rec.pitch(), -tiltAngle, tiltAngle);
    float roll = constrain(rec.roll(), -tiltAngle, tiltAngle);
    // Turn these to a joystick config!
    xyCoords xy = {
        constrain(pitch / tiltAngle, -1.0f, 1.0f),
        constrain(roll / tiltAngle, -1.0f, 1.0f)
    };
    return xy; 
}
/****************************************************************************/