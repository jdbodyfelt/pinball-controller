#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#pragma once
#include <Wire.h>

/****************************************************************************/
constexpr uint8_t SDA_PIN = 8; // Set I2C Data & Clock GPIO Pins
constexpr uint8_t SCL_PIN = 9;
constexpr uint32_t I2C_CLK = 0x61A80; // 400 Hz, High Speed!

constexpr float SMPL_FREQ = 100.f; 

/****************************************************************************/
static inline bool setWire(
    uint8_t sdaPin = SDA_PIN, 
    uint8_t sclPin = SCL_PIN, 
    uint32_t clock = I2C_CLK
){
    /*
    Let's run a harmless dummy operation check first.
    Different I2C libraries return different error codes for "not initialized".
    Typically, if begin() wasn't called, will throw some error!
    0 = success, 2 = address NACK (but bus works)
    */
    Wire.beginTransmission(0x00); // Address 0x00 is never used
    uint8_t error = Wire.endTransmission();
    if (error == 0 || error == 2)
    {
        return false;
    }                           // Already Alive!
    Wire.begin(sdaPin, sclPin); // Initialize I2C (Global Wire)
    Wire.setClock(clock);       // Set I2C comm channel (vs. 100Hz standard)
    delay(100);                 // Let I2C stabilize
    return true;
}
/****************************************************************************/
static inline uint32_t getInterval(){
    return static_cast<uint32_t>(round(1e3 / SMPL_FREQ));
}
/****************************************************************************/
static inline bool ErrorMsg(String msg, Stream& stream = Serial){ 
    stream.println(msg);
    return false; 
}
/****************************************************************************/
inline void debugPause(String msg = "\n") {
    Serial.print(msg);
    Serial.println("⏸️ PAUSED - Press any key to continue...");
    Serial.flush();
    while (!Serial.available()) {
        delay(100);
    }
    while (Serial.available()) Serial.read(); // Clear buffer
    Serial.println("▶️ RESUMING...");
    Serial.flush();
}
/****************************************************************************/
inline void msgPause(Stream& stream = Serial){ Serial.flush(); delay(100); }
/****************************************************************************/
inline void clear(Stream& stream = Serial){ 
    stream.printf("\033[2J\033[H");
    msgPause(stream);
}
/****************************************************************************/
#endif