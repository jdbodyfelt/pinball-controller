#include <Accelerometer.hpp>

/****************************************************************************/
// Object Instantiations   
Accelerometer accel;


/****************************************************************************/
void setup() {
    Serial.begin(115200);           // Serial Messaging Channel 
    Serial.println("Serial connected!");
    if(!accel.begin()){
        Serial.println("ADXL Accelerometer Not Found!");
    }
}
/****************************************************************************/
void loop() {

    static uint32_t lastUpdate = 0;
    uint32_t updateInterval = 100; 
    sensors_event_t event; 

    if (millis() - lastUpdate >= updateInterval) {
        accel.readCalibrated(&event);
        accel.tilt.print(Serial);
        lastUpdate = millis();
        delay(updateInterval);
    }
}



