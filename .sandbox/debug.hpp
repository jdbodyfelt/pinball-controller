#pragma once 
#include <Arduino.h>

inline void debugPause() {
    Serial.println("⏸️ PAUSED - Press any key to continue...");
    while (!Serial.available()) {
        delay(100);
    }
    while (Serial.available()) Serial.read(); // Clear buffer
    Serial.println("▶️ RESUMING...");
}