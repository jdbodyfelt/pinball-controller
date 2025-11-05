#pragma once
#include <Arduino.h>

/******************************************************************************/
class PulseLED {
private:
  int pin;
  bool isOn;
  unsigned long previousMillis;
  unsigned long onTime;
  unsigned long offTime;
  bool isPulsing;
  
public:
  // Constructor
  PulseLED(int ledPin, unsigned long onDuration = 500, unsigned long offDuration = 500) 
    : pin(ledPin), isOn(false), isPulsing(false), 
      onTime(onDuration), offTime(offDuration) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    previousMillis = millis();
  }

  // Turn LED on
  void on() {
    isPulsing = false;
    digitalWrite(pin, HIGH);
    isOn = true;
  }

  // Turn LED off
  void off() {
    isPulsing = false;
    digitalWrite(pin, LOW);
    isOn = false;
  }

  // Toggle LED state
  void toggle() {
    isPulsing = false;
    isOn = !isOn;
    digitalWrite(pin, isOn ? HIGH : LOW);
  }

  // Start pulsing with given durations
  void startPulse(unsigned long onDuration = 500, unsigned long offDuration = 500) {
    isPulsing = true;
    onTime = onDuration;
    offTime = offDuration;
    previousMillis = millis();
    digitalWrite(pin, HIGH);
    isOn = true;
  }

  // Stop pulsing and turn off
  void stopPulse() {
    isPulsing = false;
    digitalWrite(pin, LOW);
    isOn = false;
  }

  // Set pulse timing (without starting/stopping pulse)
  void setPulseTiming(unsigned long onDuration, unsigned long offDuration) {
    onTime = onDuration;
    offTime = offDuration;
  }

  // Get current pulse timing
  unsigned long getOnTime() { return onTime; }
  unsigned long getOffTime() { return offTime; }

  // Update method - call this in loop()
  void update() {
    if (!isPulsing) return;
    
    unsigned long currentMillis = millis();
    unsigned long elapsed = currentMillis - previousMillis;
    
    if (isOn && elapsed >= onTime) {
      digitalWrite(pin, LOW);
      isOn = false;
      previousMillis = currentMillis;
    } else if (!isOn && elapsed >= offTime) {
      digitalWrite(pin, HIGH);
      isOn = true;
      previousMillis = currentMillis;
    }
  }

  // Check if LED is on
  bool state() {
    return isOn;
  }

  // Check if currently pulsing
  bool pulsing() {
    return isPulsing;
  }
};
/******************************************************************************/