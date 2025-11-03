#pragma once
#include "Accelerometer.hpp"


// https://chat.deepseek.com/share/avz6wk2pd3j1n8n0u7

/****************************************************************************/
class Joystick : public Accelerometer {

protected: 
    float tiltAngle = 12.5f; 
    bool useDeadZone = false;           // Kills the small jiggles 
    float deadRadius = 0.1f; 
    xyCoords calibOffset = {0.0f, 0.0f};
    xyCoords currentData = {0.f, 0.f};
    bool newDataAvailable = false; 

public:
    // Call this at gamepad sampling rate
    bool update() override {
        if( Accelerometer::update() ){
            currentData = convertToJoystick(getData());
            newDataAvailable = true; 
            return true; 
        }
        newDataAvailable = false; 
        return false; 
    }


    // Configuration
    void setTiltAngle(float angle){ tiltAngle = angle; }

    // "The Dead Zone" addresses bad jittering
    void setDeadRadius(float zone){ 
        useDeadZone = zone > 0.0f; 
        deadRadius = zone; 
    }

    // Calibration
    void calibrate(float xOffset = NAN, float yOffset = NAN) {
        // Manual calibration - use provided offsets
        if (!isnan(xOffset) && !isnan(yOffset)) {
            calibOffset = {xOffset, yOffset};
            Serial.printf("📐 Manual Calibration: (X, Y) = (%.3f, %.3f)\n", xOffset, yOffset);
        } 
        // Automatic calibration to set offsets
        else {
            Serial.println("🎯 Calibrating...Hold still!");
            Serial.flush();
            const uint16_t samples = 50;
            Vec3f average;
            uint16_t validSamples = 0;
            uint32_t sampleInterval = getSampleInterval(); // From Accelerometer base class
            for (uint16_t k = 0; k < samples; k++) {
                if (update()) {
                    average = average + getData();
                    validSamples++;
                }
                delayMicroseconds(sampleInterval);
            }
            if(validSamples > 1){
                average /= static_cast<float>(validSamples);
                calibOffset = {
                    average.pitch() / tiltAngle, 
                    average.roll() / tiltAngle
                };
                Serial.printf("✅ Calibration complete: (X,Y,N) = (%.3f, %.3f, %d)\n",
                    calibOffset[0], calibOffset[1], validSamples);
            } else {
                Serial.println("❌ Calibration failed! No valid samples.");
                Serial.flush();
            }
        } 
    }
    
    // Simple read-out: what gets used the most! 
    xyCoords getXY() const { return currentData; }
    bool hasNewData() const { return newDataAvailable; }

protected:

    // Convert accelerometer XYZ to Joystick XY
    xyCoords convertToJoystick(const Vec3f data) {
        // Constrain to [-1,1]
        float pitch = constrain(data.pitch()/tiltAngle, -1.0f, 1.0f); 
        float roll = constrain(data.roll()/tiltAngle, -1.0f, 1.0f);
        // Take off offset
        pitch -= calibOffset[0];
        roll -= calibOffset[1];
        // Apply deadzone
        return applyDeadZone({pitch, roll});
    }

    // Zero out any reading in "the deadzone" and scale the rest
    xyCoords applyDeadZone(xyCoords data) {
        if(useDeadZone) {
            float mag = sqrt(data[0]*data[0] + data[1]*data[1]);
            if( mag < deadRadius ){
                return {0.0f, 0.0f};
            } else {
                float scale = (mag - deadRadius) / (1.0f - deadRadius);
                return {
                    (data[0] / mag)*scale, 
                    (data[1] / mag)*scale
                };
            }
        } else {
            return data; 
        }
    }
    
};
/****************************************************************************/