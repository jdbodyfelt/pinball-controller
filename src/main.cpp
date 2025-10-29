#include <Arduino.h>
#include <Accelerometer.hpp>

/****************************************************************************/
// Object Instantiations   
Accelerometer accel; 

/****************************************************************************/
void setup() {
    Serial.begin(115200);       // Serial Messaging Channel 
    if (!accel.begin()) {
        Serial.println("Failed to initialize accelerometer!");
        while(1);
    }
}

/****************************************************************************/
void loop() {
    accel.read();
    delay(5); // Small delay to avoid busy looping
}
