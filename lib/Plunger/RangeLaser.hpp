#ifndef RangeLaser_HPP
#define RangeLaser_HPP
#pragma once 

#include <Wire.h>
#include <VL6180X.h>
#include "utilities.hpp"

constexpr uint8_t DEVICE_ADDRESS = 0x29;

/*************************************************************************************/
class RangeLaser {
private:
  VL6180X sensor;
  String lastError;
  bool init; 
  bool continuous;
  float offset_mm; 
  
  void setError(const String& error);

public:

  RangeLaser();
  bool begin();
  bool isConnected();
  
  // Measurement modes
  void startContinuousMode(uint16_t interval_ms = 100);
  void stopContinuousMode();
  void setSingleShotMode();
  
  // Reading functions
  float readDistanceMM();                    // Single shot reading
  float readDistanceContinuous();            // Continuous mode reading
  bool isRangeComplete();                    // Check if continuous reading is ready
  
  // Calibration methods
  bool calibrateZeroOffset(uint16_t known_distance_mm = 0, uint8_t samples = 32);
  void setOffset(float offset_mm);
  float getOffset() const { return offset_mm; }
  void clearOffset() { offset_mm = 0.0f; }
  
  // Configuration
  void setScaling(uint8_t scaling);
  void setTimeout(uint16_t timeout_ms);
  
  // Status and utility
  bool isObjectInRange(float min_mm = 0, float max_mm = 200);
  bool timeoutOccurred();
  String getLastError();
  void printDiagnostics();
  
  bool isContinuousMode() { return continuous; }


};

#endif