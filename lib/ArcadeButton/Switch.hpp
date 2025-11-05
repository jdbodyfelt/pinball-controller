#pragma once
#include <Arduino.h>
#include <functional>

/******************************************************************************/
class CallbackSwitch {
private:
  int pin;
  bool lastRawState;
  bool currentState;
  bool previousState;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;
  
  // Change to std::function to support lambdas with captures
  std::function<void()> pressCallback;
  std::function<void()> releaseCallback;
  
public:
  // Constructor
  CallbackSwitch(int switchPin, unsigned long debounce = 50) 
    : pin(switchPin), lastRawState(HIGH), currentState(HIGH), previousState(HIGH),
      lastDebounceTime(0), debounceDelay(debounce) {
    pinMode(pin, INPUT_PULLUP);
  }

  // Set callback functions - now accepts any callable including lambdas
  void onPress(std::function<void()> callback) {
    pressCallback = callback;
  }
  
  void onRelease(std::function<void()> callback) {
    releaseCallback = callback;
  }

  // Update switch state and trigger callbacks
  void update() {
    bool reading = digitalRead(pin);
    
    if (reading != lastRawState) {
      lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
      previousState = currentState;
      
      if (reading != currentState) {
        currentState = reading;
        
        // Trigger callbacks
        if (currentState == LOW && pressCallback) {
          pressCallback();
        } else if (currentState == HIGH && releaseCallback) {
          releaseCallback();
        }
      }
    }
    
    lastRawState = reading;
  }

  // Basic state checks
  bool isPressed() { return currentState == LOW; }
  bool isReleased() { return currentState == HIGH; }

  // Edge detection
  bool wasPressed() {
    return (currentState == LOW) && (previousState == HIGH);
  }

  bool wasReleased() {
    return (currentState == HIGH) && (previousState == LOW);
  }

  // Get the raw pin reading
  bool rawState() { return digitalRead(pin); }

  // Get callback functions
  std::function<void()> getPressCallback() { return pressCallback; }
  std::function<void()> getReleaseCallback() { return releaseCallback; }
};
/******************************************************************************/