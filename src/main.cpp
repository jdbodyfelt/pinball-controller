#include "Solenoid.h"

// Create solenoid instance on pin 1, channel 0
Solenoid solenoid(1, 0);

void setup() {
    Serial.begin(115200);
    
    // Initialize solenoid
    if (solenoid.begin()) {
        Serial.println("Solenoid initialized successfully!");
        
        // Test sequence
        solenoid.on();  // Full power
        delay(1000);
        
        solenoid.setDutyPercentage(50.0f);  // 50% power
        delay(1000);
        
        solenoid.setDuty(1024);  // 25% power (1024/4096)
        delay(1000);
        
        solenoid.off();  // Turn off
        delay(1000);
        
        // Print status
        Serial.printf("Current duty: %d (%.1f%%)\n", 
                     solenoid.getDuty(), 
                     solenoid.getDutyPercentage());
    } else {
        Serial.println("Failed to initialize solenoid!");
    }
}

void loop() {
    // Ramp up and down
    for (int i = 0; i <= 4095; i += 100) {
        solenoid.setDuty(i);
        delay(10);
    }
    for (int i = 4095; i >= 0; i -= 100) {
        solenoid.setDuty(i);
        delay(10);
    }
}