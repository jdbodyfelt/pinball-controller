#include <Arduino.h>
#include <Joystick.hpp>

/****************************************************************************/
// Object Instantiations   
Joystick joy;  

/****************************************************************************/
void setup() {
    Serial.begin(115200);       // Serial Messaging Channel 
    while (!Serial){ delay(10); }

    if (!joy.begin()) {         
        Serial.println("Failed to initialize joystick!");
        while(1);
    }
    joy.calibrate();            
    Serial.println("Joystick ready!");
}

/****************************************************************************/
void loop() {
    static uint32_t lastUpdate = 0;
    uint32_t updateInterval = 10'000; 
    
    if (micros() - lastUpdate >= updateInterval) {
        // ⭐ Now update() does both FIFO check AND joystick conversion
        if (joy.update()) {
            // New joystick data is available
            xyCoords xy = joy.getXY();
            Serial.printf("%.3f\t%.6f\t%.6f\n", now(), xy[0], xy[1]);
        } else {
            // No new sensor data, but you still have the last reading
            xyCoords xy = joy.getXY();
            Serial.printf("%.3f\t%.6f\t%.6f\t(cached)\n", now(),  xy[0], xy[1]);
        }
        lastUpdate = micros();
    }
}



