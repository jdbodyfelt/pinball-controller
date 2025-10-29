#include "Accelerometer.hpp"

#ifdef USB_GAMEPAD
    #include <USBHIDGamepad.h>
    USBHIDGamepad usbGamepad;
#endif

#ifdef BLE_GAMEPAD  
    #include <BleGamepad.h>
    BleGamepad bleGamepad("Pinball Controller");
#endif

Accelerometer* Accelerometer::instance = nullptr;
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
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(_addr, READ_ONE_BYTE);
    return Wire.available() ? Wire.read() : 0;
} 
/****************************************************************************/
bool Accelerometer::writeRegister(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(_addr);
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
    // Setup Butterworth filters
    float lpfNorm = lpf/fs;                        
    for(uint8_t k=0; k<3; k++){
        filters[k].setLowpass(lpfNorm, bwcQ);
    }
    return true; 
}
/****************************************************************************/
vec3f Accelerometer::average(){
    uint32_t total = 1'000'000 / dtu;   // Number of samples in 1 sec. 
    vec3f avg = {0.0f, 0.0f, 0.0f};
    vec3f reading; 
    for(uint32_t k = 0; k < total; k++){
        reading = read(doFilter=true);
        avg = add(avg, reading);
        delayMicroseconds(dtu);        // Wait for the next sample! 
    }
    for(uint8_t k = 0; k < avg.size(); k++){
        avg[k] /= total; 
    }
    return avg;
}
/****************************************************************************/
std::unique_ptr<vec3f> Accelerometer::read(bool doFilter) {
    // Check if new data is ready
    if((readRegister(REG_INT_SOURCE) & INT_DATA_READY) == 0) {
        return nullptr; 
    }
    // Read 6 bytes
    Wire.beginTransmission(instance->_addr);
    Wire.write(REG_DATAX0);
    Wire.endTransmission(false);
    Wire.requestFrom(instance->_addr, READ_SIX_BYTES);
    if (Wire.available() == 6) {
        // Read raw data bytes
        uint8_t x0 = Wire.read(), x1 = Wire.read();
        uint8_t y0 = Wire.read(), y1 = Wire.read();
        uint8_t z0 = Wire.read(), z1 = Wire.read();
        // Convert to signed 16-bit integers
        int16_t rawX = (int16_t)((x1 << 8) | x0);
        int16_t rawY = (int16_t)((y1 << 8) | y0);
        int16_t rawZ = (int16_t)((z1 << 8) | z0);
        // Convert to g's and return 
        float LSBg = 256.0f;
        float X = doFilter ? filters[0].input(rawX/LSBg) : rawX/LSBg; 
        float Y = doFilter ? filters[1].input(rawY/LSBg) : rawX/LSBg; 
        float Z = doFilter ? filters[2].input(rawX/LSBg) : rawX/LSBg; 
        return std::make_unique<vec3f>(X,Y,Z);
    }
    return nullptr;
}
/****************************************************************************/
bool Accelerometer::begin()
{
    instance = this;
    // Initiate Comms & Disable FIFO / Interrupts
    Wire.begin(sda, scl);  
    writeRegister(REG_FIFO_CTL, FIFO_MODE_BYPASS); 
    writeRegister(REG_INT_ENABLE, FIFO_MODE_BYPASS);

    checkDevice();                                  // Verify an ADXL345 device
    writeRegister(REG_POWER_CTL, POWER_STANDBY);    // Set on standby for config
    setRateParams();                                // Sets all rate parameters

     // Set the Data Range (2G default)
    uint8_t options = range | FORMAT_FULL_RES | FORMAT_JUSTIFY;  
    writeRegister(REG_DATA_FORMAT, options);

    // Enable measuring & take a calibration
    writeRegister(REG_POWER_CTL, POWER_MEASURE);
    calibration = average();
    return true;
}
/****************************************************************************/
bool Accelerometer::update(){
    return true;
}
/****************************************************************************/