// DSP Butterworth Filter 
// https://en.wikipedia.org/wiki/Butterworth_filter
#pragma once

#include "vectors.hpp"

/*************************************************/
class ButterworthFilter {
private:
    Vec3f a, b;                   // Filter Coeffs
    float x1, x2, y1, y2;         // Filter Stores 

public:
    ButterworthFilter(){}
    
    void set(float sampleFreq, float freqCutoff = 8.f, float resQ = sqrt(2.f)) {
        // freqCutoff is the cutoff frequency in Hz. 
        // sampleRate is the sampling rate in Hz. 
        // resQ is the resonant Q-factor of the filter.
        if( freqCutoff > 0.4f * sampleFreq){
            Serial.printf(
                "WARN: Nyquist invalid %.1f > 0.4 * %.1f\n", 
                freqCutoff, sampleFreq
            );
        }
        float omega = M_TWOPI * freqCutoff / sampleFreq;
        float alpha = sin(omega) / (2.0 * resQ);
        float cos_omega = cos(omega);
        b = {
            (1.f - cos_omega) / 2.f,
            1.f - cos_omega,
            (1.f - cos_omega) / 2.f
        };
        a = {
             1.f + alpha,
            -2.f * cos_omega,
            1.f - alpha
        };
        b /= a[0];   // Normalize
        a /= a[0]; 
    }

    float filter(float input) {
        float output = b[0] * input; 
        output += b[1] * x1 + b[2] * x2;
        output -=  a[1] * y1 - a[2] * y2;

        // Shift delay lines
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
        
        return output;
    }
    
    void reset() {
        x1 = x2 = y1 = y2 = 0.f;
    }
};