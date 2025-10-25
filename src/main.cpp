#include <Arduino.h>
#include <TiltSensor.hpp>

TiltSensor tiltSensor(8, 9, 1, 2); // SDA, SCL, INT1, INT2

static const float FREQ = 3.13; // Sampling frequency in Hz

void setup() {
    Serial.begin(115200);
    
    if (!tiltSensor.begin(FREQ)) {
        Serial.println("Failed to initialize tilt sensor!");
        while(1);
    }
    
    Serial.println("Tilt Sensor initialized successfully!");
    Serial.print("Reading at ");
    Serial.print(FREQ);
    Serial.println(" Hz with interrupts...");
}

void loop() {
    tiltSensor.update(); // Handles everything: reading + interrupts + printing
    

    // Change rate dynamically if needed
    /*
    static unsigned long lastChange = 0;
    if (millis() - lastChange > 5000) { // Every 5 seconds
        tiltSensor.setSamplingRate(25.0); // Switch to 25Hz
        lastChange = millis();
    }
    */

    // Other tasks

    delay(1); // Small delay to avoid busy looping
}