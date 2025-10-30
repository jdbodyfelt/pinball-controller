#include <Arduino.h>
#include <Accelerometer.hpp>

/****************************************************************************/
// Object Instantiations   
Accelerometer accel; 

/****************************************************************************/
void setup() {
    Serial.begin(115200);       // Serial Messaging Channel 
    while (!Serial){ delay(10); }
    if (!accel.begin()) {
        Serial.println("Failed to initialize accelerometer!");
        while(1);
    }

}

/****************************************************************************/
void loop() {
    float tic = now();
    xyCoords xy = accel.toJoystick();
    Serial.printf("%.3f\t%.6f\t%.6f\n",tic,xy[0],xy[1]);
    delayMicroseconds(5); // Small delay to avoid busy looping
}
