#include <Arduino.h>
#include <TiltSensor.hpp>

/****************************************************************************/
// Object Instantiations   
static const float FREQ = 1.0f; // Sampling frequency in Hz 
TiltSensor tiltSensor(8, 9, 1, 2); // SDA, SCL, INT1, INT2

/****************************************************************************/
void setup() {
    Serial.begin(9600);       // Plotter Channel  
    if (!tiltSensor.begin(FREQ)) {
        Serial.println("Failed to initialize tilt sensor!");
        while(1);
    }
}

/****************************************************************************/
void loop() {
    tiltSensor.update(); 
    delay(5); // Small delay to avoid busy looping
}
