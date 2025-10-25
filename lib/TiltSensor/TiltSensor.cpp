#include "TiltSensor.hpp"
#include <Arduino.h>

TiltSensor* TiltSensor::instance = nullptr;

TiltSensor::TiltSensor(uint8_t sda, uint8_t scl, uint8_t int1, uint8_t int2) 
    : sdaPin(sda), sclPin(scl), int1Pin(int1), int2Pin(int2), 
      sampleInterval(20), lastSampleTime(0),    // Default 50Hz (1000/50 = 20ms)
      tiltStartTime(0), tiltWarningActive(false),
      int1Triggered(false), int2Triggered(false) {
}

bool TiltSensor::begin(float samplingFrequency) {
    // Set static instance for ISR & sampling rate
    instance = this;      
    setSamplingRate(samplingFrequency);
    // Initialize pins
    Wire.begin(sdaPin, sclPin);
    pinMode(int1Pin, INPUT);        
    pinMode(int2Pin, INPUT);
    // Attach interrupts
    attachInterrupt(digitalPinToInterrupt(int1Pin), handleINT1, RISING);
    attachInterrupt(digitalPinToInterrupt(int2Pin), handleINT2, RISING);
    // Check device ID
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(REG_DEVID);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    Wire.requestFrom(ADXL345_ADDR, READ_ONE_BYTE);
    if (!Wire.available()) {
        return false;
    }
    byte deviceId = Wire.read();
    if (deviceId != 0xE5) {
        return false;
    }

    // Configure ADXL345
    // No need to set REG_BW_RATE - setSamplingRate already does!
    writeRegister(REG_POWER_CTL, 0x08);      // Measurement mode
    writeRegister(REG_DATA_FORMAT, 0x0B);    // ±16g, full resolution
    
    // Give some output feedback
    Serial.print("TiltSensor initialized at ");
    Serial.print(samplingFrequency);
    Serial.println(" Hz");
    
    return true;
}

TiltEvent TiltSensor::update() {
    // Check if it's time to sample
    unsigned long currentTime = millis();
    TiltEvent event = TiltEvent::NONE;
    // Check hardware interrupts first
    if (int1Triggered) {
        int1Triggered = false;
        Serial.println("🚨 INT1 TRIGGERED!");
        readAndPrintInterruptSource();
    }
    if (int2Triggered) {
        int2Triggered = false;
        Serial.println("🚨 INT2 TRIGGERED!");
        readAndPrintInterruptSource();
    }
    // Then only sample at specified intervals
    if (currentTime - lastSampleTime >= sampleInterval) {
        float x, y, z;
        readAcceleration(x, y, z);
        Serial.print("ACCEL: ");
        Serial.print(x);
        Serial.print("\t");
        Serial.print(y);
        Serial.print("\t");
        Serial.println(z);
        // Detect any events
        event = detectEvent(x, y, z);
        if (event != TiltEvent::NONE) {
            printEvent(event);
        }
        lastSampleTime = currentTime;
    }
    return event;
}

void TiltSensor::setSamplingRate(float frequency) {
    // Clamp frequency to valid range (0.1Hz to 100Hz for safe operation)
    frequency = constrain(frequency, 0.1, 100.0);
    
    // Convert frequency to sample interval in milliseconds
    sampleInterval = (unsigned long)(1000.0 / frequency);
    
    // Set ADXL345 data rate (choose the closest supported rate)
    uint8_t rateCode = frequencyToRateCode(frequency);
    setDataRate(rateCode);
    
    Serial.print("Sampling rate set to ");
    Serial.print(frequency);
    Serial.print(" Hz (interval: ");
    Serial.print(sampleInterval);
    Serial.println(" ms)");
}

uint8_t TiltSensor::frequencyToRateCode(float frequency) {
    // Map frequency to closest ADXL345 data rate
    if (frequency >= 1600) return RATE_3200_HZ;
    if (frequency >= 800) return RATE_1600_HZ;
    if (frequency >= 400) return RATE_800_HZ;
    if (frequency >= 200) return RATE_400_HZ;
    if (frequency >= 100) return RATE_200_HZ;
    if (frequency >= 50) return RATE_100_HZ;
    if (frequency >= 25) return RATE_50_HZ;
    if (frequency >= 12.5) return RATE_25_HZ;
    if (frequency >= 6.25) return RATE_12_5_HZ;
    if (frequency >= 3.13) return RATE_6_25_HZ;
    if (frequency >= 1.56) return RATE_3_13_HZ;
    if (frequency >= 0.78) return RATE_1_56_HZ;
    if (frequency >= 0.39) return RATE_0_78_HZ;
    if (frequency >= 0.20) return RATE_0_39_HZ;
    if (frequency >= 0.10) return RATE_0_20_HZ;
    return RATE_0_10_HZ;
}

bool TiltSensor::setDataRate(uint8_t rateCode) {
    return writeRegister(REG_BW_RATE, rateCode);
}

TiltEvent TiltSensor::detectEvent(float x, float y, float z) {
    // Calculate acceleration magnitudes
    float verticalBump = fabs(z - 9.8);
    float lateralAccel = fmax(fabs(x), fabs(y));
    float totalAccel = sqrt(x*x + y*y + z*z);

    // Detect free fall (machine lifted or falling)
    if (totalAccel < freeFallThreshold) { 
        tiltWarningActive = false;
        return TiltEvent::FREE_FALL;
    }
    // Detect slam tilt (hard impact)
    if (verticalBump > slamTiltThreshold || lateralAccel > slamTiltThreshold) {
        tiltWarningActive = false;
        return TiltEvent::SLAM_TILT;
    }   
    // Detect bump (quick nudge)
    if (verticalBump > bumpThreshold || lateralAccel > bumpThreshold) {
        return TiltEvent::BUMP;
    }
    // Detect sustained tilt
    if (lateralAccel > tiltAccelThreshold) {
        if (!tiltWarningActive) {
            tiltStartTime = millis();
            tiltWarningActive = true;
            return TiltEvent::TILT_WARNING;
        } else if (millis() - tiltStartTime >= tiltTimeThreshold) {
            tiltWarningActive = false;
            return TiltEvent::TILT;
        }
    } else {
        if (tiltWarningActive) {
            tiltWarningActive = false;
            return TiltEvent::STABLE;
        }
    }
    return TiltEvent::NONE;
}

void TiltSensor::printEvent(TiltEvent event) {
    switch(event) {
        case TiltEvent::BUMP:
            Serial.println("💥 BUMP detected!");
            break;
        case TiltEvent::TILT_WARNING:
            Serial.println("⚠️ TILT WARNING!");
            break;
        case TiltEvent::TILT:
            Serial.println("🚫 TILT! Penalty applied.");
            break;
        case TiltEvent::SLAM_TILT:
            Serial.println("💫 SLAM TILT! Immediate penalty.");
            break;
        case TiltEvent::FREE_FALL:
            Serial.println("🔽 FREE FALL detected!");
            break;
        case TiltEvent::STABLE:
            Serial.println("✅ Back to STABLE.");
            break;
        default:
            break;
    }
}   

float TiltSensor::getCurrentSamplingRate() {
    return 1000.0 / sampleInterval;
}

void TiltSensor::readAcceleration(float &x, float &y, float &z) {
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(REG_DATAX0);
    if (Wire.endTransmission() != 0) {
        x = y = z = 0;
        return;
    }
    Wire.requestFrom(ADXL345_ADDR, READ_SIX_BYTES);
    if (Wire.available() == 6) {
        int16_t rawX = (Wire.read() | (Wire.read() << 8));
        int16_t rawY = (Wire.read() | (Wire.read() << 8));
        int16_t rawZ = (Wire.read() | (Wire.read() << 8));
        
        // Convert to m/s² (±16g range, 1g = 9.8 m/s²)
        x = (rawX / 256.0) * 9.8;
        y = (rawY / 256.0) * 9.8;
        z = (rawZ / 256.0) * 9.8;
    } else {
        x = y = z = 0;
    }
}

bool TiltSensor::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

uint8_t TiltSensor::readRegister(uint8_t reg) {
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(ADXL345_ADDR, READ_ONE_BYTE);
    return Wire.available() ? Wire.read() : 0;
}   

void TiltSensor::readAndPrintInterruptSource() {
    uint8_t intSource = readRegister(REG_INT_SOURCE);
    Serial.print("Interrupt Source: 0x");
    Serial.print(intSource, HEX);
    Serial.print(" (");
    bool first = true;
    const char* sources[] = {
        "OVERRUN", "WATERMARK", "FREE_FALL", "INACTIVITY",
        "ACTIVITY", "DOUBLE_TAP", "SINGLE_TAP", "DATA_READY"
    };
    for (int i = 0; i < 8; i++) {
        if (intSource & (1 << i)) {
            if (!first) Serial.print(" | ");
            Serial.print(sources[7-i]);
            first = false;
        }
    }
    Serial.println(")");
}

void TiltSensor::handleINT1() {
    if (instance) {
        instance->int1Triggered = true;
    }
}

void TiltSensor::handleINT2() {
    if (instance) {
        instance->int2Triggered = true;
    }   
}