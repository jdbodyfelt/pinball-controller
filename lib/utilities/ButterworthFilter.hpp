// DSP Butterworth Filter 
// https://en.wikipedia.org/wiki/Butterworth_filter
#pragma once

/*************************************************/
class ButterworthFilter {
private:
    float b0, b1, b2, a1, a2;
    float x1, x2, y1, y2;
    
public:
    ButterworthFilter() : 
    b0(1), b1(0), b2(0), 
    a1(0), a2(0), 
    x1(0), x2(0), 
    y1(0), y2(0) {}
    
    void setLowpass(float freqCutoff, float resQ, float sampleRate) {
        float omega = TWO_PI * freqCutoff / sampleRate;
        float alpha = sin(omega) / (2.0 * resQ);
        float cos_omega = cos(omega);
        
        b0 = (1.0 - cos_omega) / 2.0;
        b1 = 1.0 - cos_omega;
        b2 = (1.0 - cos_omega) / 2.0;
        float a0 = 1.0 + alpha;
        a1 = -2.0 * cos_omega;
        a2 = 1.0 - alpha;
        
        // Normalize coefficients
        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;
    }
    
    float process(float input) {
        float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

        // Shift delay lines
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
        
        return output;
    }
    
    void reset() {
        x1 = x2 = y1 = y2 = 0;
    }
};