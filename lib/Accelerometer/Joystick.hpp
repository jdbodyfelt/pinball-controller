#ifndef JOYSTICK_HPP
#define JOYSTICK_HPP

#pragma once
#include "Accelerometer.hpp"

/****************************************************************************/
class Joystick {

private: 
    Accelerometer& accel;

    float _maxTilt;                 // Maximum Deg. of Tilt
    Vec2f _tilt;                    // This is my pitch & roll! 
    Vec2f _zero;                    // Tilt Average for DC Offet to Zero
    bool _init; 
 
    bool readRaw(); 
    bool readCalibrated(); 
    bool readClipped(); 

public:

    Joystick(Accelerometer& accelerometer, float maxTilt = 12.5f);

    bool begin();
    Vec2f read();
    bool calibrate(Vec2f manual = Vec2f(NAN));
    void print(Stream& stream);
};
/****************************************************************************/

#endif