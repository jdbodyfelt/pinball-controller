#include "RangeLaser.hpp"

/*************************************************************************************/
RangeLaser::RangeLaser() : init(false), continuous(false)
{
}
/*************************************************************************************/
bool RangeLaser::begin()
{
    setWire();  // Initialize I2C
    delay(100); // Allow sensor to power up
    // Check if sensor exists first
    Wire.beginTransmission(DEVICE_ADDRESS);
    if (Wire.endTransmission() != 0)
    {
        setError("Not found at address 0x" + String(DEVICE_ADDRESS, HEX));
        return false;
    }
    // Do some initialisers
    sensor.setAddress(DEVICE_ADDRESS);
    sensor.init();
    sensor.configureDefault();
    sensor.setTimeout(500);
    sensor.stopContinuous(); // Ensure we start in single shot mode
    delay(100);

    continuous = false;
    init = true;
    setError("");
    return true;
}
/*************************************************************************************/
bool RangeLaser::isConnected()
{
    Wire.beginTransmission(sensor.getAddress());
    return (Wire.endTransmission() == 0);
}
/*************************************************************************************/
void RangeLaser::startContinuousMode(uint16_t interval_ms)
{
    if (!init)
        return;
    sensor.startRangeContinuous(interval_ms);
    continuous = true;
}
/*************************************************************************************/
void RangeLaser::stopContinuousMode()
{
    if (!init)
        return;
    sensor.stopContinuous();
    continuous = false;
    delay(100);
}
/*************************************************************************************/
void RangeLaser::setSingleShotMode()
{
    stopContinuousMode();
}
/*************************************************************************************/
float RangeLaser::readDistanceMM()
{
    if (!init)
    {
        setError("Sensor not initialized");
        return -1.0;
    }
    // Ensure we're in single shot mode
    if (continuous)
    {
        stopContinuousMode();
    }
    // Single shot reading
    uint16_t distance = sensor.readRangeSingleMillimeters();
    if (sensor.timeoutOccurred())
    {
        setError("Timeout during reading");
        return -1.0;
    }
    if (distance == 65535)
    {
        setError("Out of range");
        return -1.0;
    }
    setError("");
    return (float)distance;
}
/*************************************************************************************/
float RangeLaser::readDistanceContinuous()
{
    if (!init)
    {
        setError("Sensor not initialized");
        return -1.0;
    }
    if (!continuous)
    {
        setError("Not in continuous mode");
        return -1.0;
    }
    uint16_t distance = sensor.readRangeContinuousMillimeters();
    if (sensor.timeoutOccurred())
    {
        setError("Sensor timeout during continuous reading");
        return -1.0;
    }
    if (distance == 65535)
    {
        setError("Continuous measurement out of range");
        return -1.0;
    }
    setError("");
    return (float)distance + offset_mm;     // Apply calibration offset
}
/*************************************************************************************/
bool RangeLaser::calibrateZeroOffset(uint16_t known_distance_mm, uint8_t samples) {
  if (!init) {
    setError("Sensor not initialized for calibration");
    return false;
  }
  
  if (continuous) {
    stopContinuousMode();
  }
  
  Serial.println("Starting zero offset calibration...");
  Serial.println("Ensure sensor has clear view of target at known distance: " + String(known_distance_mm) + "mm");
  Serial.println("Taking " + String(samples) + " samples...");
  
  float sum = 0;
  uint8_t valid_samples = 0;
  
  for (int i = 0; i < samples; i++) {
    float distance = readDistanceMM();
    if (getLastError().length() == 0 && distance != 65535 && distance > 0) {
      sum += distance;
      valid_samples++;
      Serial.println("Sample " + String(i+1) + ": " + String(distance, 1) + "mm");
    } else {
      Serial.println("Sample " + String(i+1) + ": Invalid reading");
    }
    delay(50); // Small delay between samples
  }
  if (valid_samples < samples * 0.7) { // Require at least 70% valid samples
    setError("Calibration failed: Too many invalid readings (" + String(valid_samples) + "/" + String(samples) + " valid)");
    return false;
  }
  float average_distance = sum / valid_samples;
  offset_mm = known_distance_mm - average_distance;
  Serial.println("Calibration complete:");
  Serial.println("  Average measured: " + String(average_distance, 1) + "mm");
  Serial.println("  Known distance: " + String(known_distance_mm) + "mm");
  Serial.println("  Calculated offset: " + String(offset_mm, 1) + "mm");
  setError("");
  return true;
}
/*************************************************************************************/
void RangeLaser::setOffset(float offset_mm) {
  this->offset_mm = offset_mm;
  Serial.println("Offset set to: " + String(offset_mm, 1) + "mm");
}
/*************************************************************************************/
void RangeLaser::printDiagnostics() {
  Serial.println("=== VL6180X Range Laser Sensor Diagnostics ===");
  Serial.println("Initialized: " + String(init ? "Yes" : "No"));
  Serial.println("I2C Address: 0x" + String(sensor.getAddress(), HEX));
  Serial.println("I2C Pins: SDA=" + String(SDA_PIN) + ", SCL=" + String(SCL_PIN));
  Serial.println("Mode: " + String(continuous ? "Continuous" : "Single Shot"));
  Serial.println("Connected: " + String(isConnected() ? "Yes" : "No"));
  Serial.println("Calibration Offset: " + String(offset_mm, 1) + "mm");
  Serial.println("Last Error: " + lastError);
  Serial.println("=================================");
}
/*************************************************************************************/
bool RangeLaser::isRangeComplete()
{
    return init && continuous;
}
/*************************************************************************************/
void RangeLaser::setScaling(uint8_t scaling)
{
    if (init)
    {
        sensor.setScaling(scaling);
    }
}
/*************************************************************************************/
void RangeLaser::setTimeout(uint16_t timeout_ms)
{
    if (init)
    {
        sensor.setTimeout(timeout_ms);
    }
}
/*************************************************************************************/
bool RangeLaser::isObjectInRange(float min_mm, float max_mm)
{
    float distance = continuous ? readDistanceContinuous() : readDistanceMM();
    return (distance >= min_mm && distance <= max_mm);
}
/*************************************************************************************/
bool RangeLaser::timeoutOccurred()
{
    return init ? sensor.timeoutOccurred() : false;
}
/*************************************************************************************/
String RangeLaser::getLastError()
{
    return lastError;
}
/*************************************************************************************/
void RangeLaser::setError(const String &error)
{
    lastError = error;
    if (error.length() > 0)
    {
        Serial.println("VL6180X ERROR: " + error);
    }
}
/*************************************************************************************/