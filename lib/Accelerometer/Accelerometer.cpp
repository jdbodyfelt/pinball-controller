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
    // Check device is registered
    Wire.write(devId);
    if( Wire.endTransmission() != 0 ) {
        Serial.printf("%s Bad Write to Device ID: %02X\n", critMsg, devId); 
        return false;
    }
    // Check the I2C Bus
    Wire.requestFrom(addr, READ_ONE_BYTE);
    if (!Wire.available()) {
        Serial.printf("%s I2C Read Failure on: %02X\n", critMsg, addr); 
        return false;
    }
    devId = Wire.read();
    if ( devId != 0xE5 ) {
        Serial.printf("%s Bad Read from Device ID: %02X\n", critMsg, devId); 
        return false; 
    }
    return true; 
}
/****************************************************************************/
bool Accelerometer::setRateParams() {
    // Set the data rate register
    if ( !writeRegister(REG_BW_RATE, rate) ) {
        Serial.printf("%s Bad rate code: %02X\n", critMsg, rate);
        return false; 
    }
    // Define intervals from the rate code
    float fs = getFrequency(rate);
    dt = 1.0f / fs;                             // Interval Period
    dtu = static_cast<uint32_t>(1e6*dt);        // Period in microsec

    // Setup Butterworth filters                     
    for(uint8_t k=0; k<3; k++){
        _filts[k].setLowpass(lpf, bwcQ, fs);
    }
    return true; 
}
/****************************************************************************/
Vec3f Accelerometer::read() {
    Wire.beginTransmission(addr);
    Wire.write(REG_DATAX0);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, READ_SIX_BYTES);
    // Take a snapshot and return it. 
    if (Wire.available() == 6) {
        // Read raw data bytes
        uint8_t x0 = Wire.read(), x1 = Wire.read();
        uint8_t y0 = Wire.read(), y1 = Wire.read();
        uint8_t z0 = Wire.read(), z1 = Wire.read();
        // Convert to signed 16-bit integers
        int16_t rawX = (int16_t)((x1 << 8) | x0);
        int16_t rawY = (int16_t)((y1 << 8) | y0);
        int16_t rawZ = (int16_t)((z1 << 8) | z0);
        Vec3f snapshot(rawX, rawY, rawZ);
        //snapshot.filter(_filts);            // Apply Butterworth filtering
        snapshot /= 256.0f;                 // Convert to g's, LSBs / g
        return snapshot;
    }
    else {
        Serial.println("🚨 FAIL: Wire.read()");
        return lastData; 
    }
}
/****************************************************************************/
bool Accelerometer::begin()
{
    
    Wire.begin(sda, scl);                           // Initiate Comms  
    if(!checkDevice()) { return false; }            // Verify an ADXL345 device                       
    writeRegister(REG_POWER_CTL, POWER_STANDBY);    // Standby for config
    
    if(!setRateParams()) { return false; }          // Sets rate parameters
                             
    uint8_t options = range | FORMAT_FULL_RES;      // Define format & range 
    writeRegister(REG_DATA_FORMAT, options);        // Set format & range

    writeRegister(REG_FIFO_CTL, FIFO_MODE_BYPASS);  // Bypass FIFO for immediate reads
    writeRegister(REG_INT_ENABLE, 0);               // Disable all interrupts (we poll FIFO status)
    
    writeRegister(REG_POWER_CTL, POWER_MEASURE);    // Enable measure-ready
    dataValid = false;      

    Serial.printf("✅ ADXL345 Running @ %d Hz (%d μs)\n", getFrequency(rate), dtu);
    return true;
}
/****************************************************************************/
bool Accelerometer::update() {
    // Check if data is ready (non-blocking)
    if ((readRegister(REG_INT_SOURCE) & INT_DATA_READY) == 0) {
        return false; // No new data available!
    }
    // Read the data immediately
    lastData = read();
    dataValid = true;
    return true;
}
/****************************************************************************/
