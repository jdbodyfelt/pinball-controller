#include "Solenoid.hpp"

Solenoid solenoid(1, 0);

void setup() {
    Serial.begin(115200);
    solenoid.begin();
    
    // Simple on/off test - no PWM
    Serial.println("TEST: Turning solenoid ON");
    digitalWrite(1, HIGH);  // Bypass PWM, force pin high
    delay(2000);
    
    Serial.println("TEST: Turning solenoid OFF"); 
    digitalWrite(1, LOW);   // Force pin low
    delay(2000);
}

void loop() {
    // Your 30% duty cycle test
    Serial.println("30% duty cycle for 3s");
    solenoid.setDutyPercentage(30.0f);
    Serial.printf("Duty set to: %d (%.1f%%)\n", solenoid.getDuty(), solenoid.getDutyPercentage());
    delay(3000);
    
    solenoid.off();
    Serial.println("OFF for 10s");
    delay(10000);
}