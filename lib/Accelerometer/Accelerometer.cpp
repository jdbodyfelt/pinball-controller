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
    float lowPassCutoff, float highPassCutoff, float maxTiltAngle
) : 
    _range(dataRange), _rate(dataRate),
    _lpf(lowPassCutoff), _hpf(highPassCutoff), _tiltAngle(maxTiltAngle)
 {}
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
    Wire.requestFrom(_addr, READ_ONE_BYTE);
    if (!Wire.available()) {
        Serial.printf("%s I2C Read Failure on: %02X\n", msg, _addr); 
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
bool Accelerometer::begin()
{
    instance = this;
    // Initiate Comms & Disable FIFO / Interrupts
    Wire.begin(_sda, _scl);  
    writeRegister(REG_FIFO_CTL, FIFO_MODE_BYPASS); 
    writeRegister(REG_INT_ENABLE, FIFO_MODE_BYPASS);

    checkDevice();                                  // Verify an ADXL345 device
    writeRegister(REG_POWER_CTL, POWER_STANDBY);    // Set on standby for config
    setDataRate();                                  // Data Rate (100 Hz default)
     // Set the Data Range (2G default)
    uint8_t options = _range | FORMAT_FULL_RES | FORMAT_JUSTIFY;  
    writeRegister(REG_DATA_FORMAT, options);
    // Enable measuring
    writeRegister(REG_POWER_CTL, POWER_MEASURE);
    return true;
}
/****************************************************************************/
void Accelerometer::read() {
    // Check if new data is ready
    if((readRegister(REG_INT_SOURCE) & INT_DATA_READY) == 0) {
        return; // No new data
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
        // Send for further processing
        float time = 1e-6f * micros();
        instance->process(time, rawX, rawY, rawZ);
    }
}
/****************************************************************************/
vec3f Accelerometer::lowpass(const vec3f &vals) {
    float alfa = _dt / (1.0/(TWO_PI*_lpf) + _dt); 
    for (uint8_t k = 0; k < 3; k++) {
        _lpf_in[k] = alfa * vals[k] + (1.0f-alfa) * _lpf_in[k];
    }
    return _lpf_in; 
}
/****************************************************************************/
vec3f Accelerometer::highpass(const vec3f &vals) {
    float alfa = 1.0/(TWO_PI*_hpf) / (_dt + 1.0/(TWO_PI*_hpf));
    for (uint8_t k = 0; k < 3; k++) {
        _hpf_out[k] = alfa * (_hpf_out[k] + vals[k] - _hpf_in[k]);
        _hpf_in[k] = vals[k];
    }
    return _hpf_out;
}
/****************************************************************************/

void Accelerometer::to_joystick(float time, int16_t x, int16_t y, int16_t z) {
    // Convert to g's and do IIR filtering
    vec3f input_g = {x/256.0f, y/256.0f, z/256.0f};
    auto low = lowpass(input_g);
    auto high = highpass(input_g);
    // Convert accel to tilt angles
    float joy_x = atan2(low[0], low[2]) * RAD_TO_DEG / _tiltAngle;
    float joy_y = atan2(low[1], low[2]) * RAD_TO_DEG / _tiltAngle;
    // Constrain result
    joy_x = constrain(joy_x, -1.0f, 1.0f);
    joy_y = constrain(joy_y, -1.0f, 1.0f);
    // Serial
    Serial.printf("%.4f\t%.4f\t%.4f\n", time, joy_x, joy_y);
}
/****************************************************************************/
bool Accelerometer::writeRegister(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.write(val);
    return (Wire.endTransmission() == 0);
}
/****************************************************************************/
uint8_t Accelerometer::readRegister(uint8_t reg) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(_addr, READ_ONE_BYTE);
    return Wire.available() ? Wire.read() : 0;
} 
/****************************************************************************/
bool Accelerometer::setDataRate() {
    // Set the data rate register
    if ( !writeRegister(REG_BW_RATE, _rate) ) {
        char msg[] = "🚨 ADXL345 CRITICAL!";
        Serial.printf("%s Bad rate code: %02X\n", msg, _rate);
        return false; 
    }
    _dt = 1.0f / getFrequency(_rate);           // Define the period from the rate
    _dtu = static_cast<uint32_t>(1e6 * _dt);    // Period in microsec
    return true; 
}
/****************************************************************************/