#include <Joystick.hpp>

/****************************************************************************/
// Object Instantiations   
Accelerometer accel;
Joystick joy(accel); 

uint32_t lastUpdate = 0;
const uint32_t updateInterval = getInterval();

/****************************************************************************/
void setup() {
    Serial.begin(115200);           // Serial Messaging Channel 
    Serial.println("Serial connected!");
    if(!joy.begin()){
        Serial.println("Joystick Emulator Not Found!");
    }
    joy.calibrate();
    Serial.flush(); delay(700);
    clear(Serial);
}
/****************************************************************************/
void loop() {
    if (millis() - lastUpdate >= updateInterval) {
        joy.print(Serial);
        lastUpdate = millis();
        delay(updateInterval);
    }
}



