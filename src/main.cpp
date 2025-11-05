#include "RangeLaser.hpp"

RangeLaser plunger;

void setup() {
  Serial.begin(115200);
  
  if (!plunger.begin()) {
    Serial.println("Plunger initialization failed!");
    while(1);
  }

  /**/
  // Calibrate with known distance (e.g., sensor mounted 10mm from reference surface)
  if (plunger.calibrateZeroOffset(10, 20)) { // 20 samples at 10mm known distance
    Serial.println("Calibration successful!");
  } else {
    Serial.println("Calibration failed: " + plunger.getLastError());
  }
}
  
  plunger.startContinuousMode(50); // 50ms between measurements
}

void loop() {
  // In Pololu continuous mode, we can read at any time
  // The sensor handles the timing internally
  float distance = plunger.readDistanceContinuous();
  
  if (plunger.getLastError().length() == 0) {
    Serial.println("Distance: " + String(distance) + " mm");
  }
  
  delay(10); // Small delay to prevent serial overflow
}