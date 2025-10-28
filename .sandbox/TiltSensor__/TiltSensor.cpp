#include "TiltSensor.hpp"
#include <Arduino.h>

TiltSensor* TiltSensor::instance = nullptr;

/****************************************************************************/
TiltSensor::TiltSensor(
    uint8_t sda, uint8_t scl, uint8_t int1, 
    uint8_t int2, uint16_t sampleIntMs
) : 
sdaPin(sda), sclPin(scl), int1Pin(int1), int2Pin(int2), 
sampleInterval(sampleIntMs), lastSampleTime(0),
tiltStartTime(0), tiltWarningActive(false),
int1Triggered(false), int2Triggered(false) {
}

/****************************************************************************/
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
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(DEV_ID);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    // Verify I2C COM
    Wire.requestFrom(I2C_ADDR, READ_ONE_BYTE);
    if (!Wire.available()) {
        return false;
    }
    byte deviceId = Wire.read();
    if (deviceId != 229) {
        return false;
    }
    // Configure ADXL345
    // No need to set BW_RATE - setSamplingRate already does!
    writeRegister(PWR_CTL, 8);      // Measurement mode
    writeRegister(DATA_FMT, 11);    // ±16g, full resolution
    
    // Give some output feedback
    /*
    Serial.print("TiltSensor initialized @ ");
    Serial.print(samplingFrequency);
    Serial.println(" Hz");
    */
    return true;
}


/****************************************************************************/
void TiltSensor::update() {
    // Check if it's time to sample!
    uint32_t currentTime = millis();
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
        Vector accel = readAcceleration();
        // Adjust for calibration offsets
        //accel -= calibrationAccel;
        // Detect events based on adjusted acceleration
        /*
        event = detectEvent(accel - calibrationAccel);
        if (event != TiltEvent::NONE) {
            printEvent(event);
            delay(500); // Small delay to catch an eye on the event
        } else {
            accel.print(Serial, true, false); 
        }
        */
       accel.print(Serial, true);
        
    }
    return;
}

/****************************************************************************/
bool TiltSensor::setSamplingRate(float frequency) {
    // Clamp frequency to valid range (0.1Hz to 100Hz for safe operation)
    frequency = constrain(frequency, 0.1, 100.0);
    // Convert frequency to sample interval in milliseconds
    sampleInterval = (unsigned long)(1000.0 / frequency);
    // Set ADXL345 data rate (choose the closest supported rate)
    uint8_t rateCode = frequencyToRateCode(frequency);
    // Set the data rate register
    if (!writeRegister(BW_RATE, rateCode)) {
        Serial.print("FAIL: BW_RATE register unable to set with rate code ");
        Serial.print(rateCode);
        Serial.print(" and frequency of ");
        Serial.print(frequency, 2);
        Serial.println(" Hz.");
        return false; 
    } else {
        Serial.print("Sampling rate (BW_RATE) register set to ");
        Serial.print(frequency, 2);
        Serial.print(" Hz (interval: ");
        Serial.print(sampleInterval);
        Serial.print(" ms, rate code: "); 
        Serial.println(rateCode, HEX);
        return true;
    }
};

/****************************************************************************/
uint8_t TiltSensor::frequencyToRateCode(float frequency) {
    // Map frequency to closest ADXL345 data rate via bounds checks
    std::array<float,15> frequencyBounds = {
        0.1, 0.2, 0.39, 0.78, 
        1.56, 3.13, 6.25, 12.5, 
        25, 50, 100, 200, 400, 800, 1600
    };
    for (int i = frequencyBounds.size() - 1; i >= 0; i--) {
        if (frequency > frequencyBounds[i]) {
            return i + 1;
        }
    }
    return 0;
}

/****************************************************************************/
TiltEvent TiltSensor::detectEvent(Vector accel) {
    // Calculate acceleration magnitudes
    float vertBump = fabs(accel.z);
    float latAccel = fmax(accel.x, accel.y);
    float accelMag = accel.magnitude();
    // Detect free fall (machine lifted or falling)
    if (accelMag < freeThreshold) { 
        tiltWarningActive = false;
        return TiltEvent::FREE_FALL;
    }
    // Detect slam tilt (hard impact)
    if (vertBump > slamThreshold || latAccel > slamThreshold) {
        tiltWarningActive = false;
        return TiltEvent::SLAM_TILT;
    }   
    // Detect bump (quick nudge)
    if (vertBump > bumpThreshold || latAccel > bumpThreshold) {
        return TiltEvent::BUMP;
    }
    // Detect sustained tilt
    if (latAccel > tiltThreshold) {
        if (!tiltWarningActive) {
            tiltStartTime = millis();
            tiltWarningActive = true;
            return TiltEvent::TILT_WARNING;
        } else if (millis() - tiltStartTime >= tiltTime) {
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

/****************************************************************************/
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

/****************************************************************************/
Vector TiltSensor::readAcceleration() {
    float err = -1024.0f;
    Wire.beginTransmission(I2C_ADDR);
    // Set the endian for reading - start at DATA_X0
    Wire.write(DATA_X0);
    if (Wire.endTransmission() != 0) {
        Vector vals = Vector(err, err, err);
        return vals;
    }
    Wire.requestFrom(I2C_ADDR, READ_SIX_BYTES);
    if (Wire.available() == 6) {
        // Read raw data bytes
        uint8_t x0 = Wire.read();
        uint8_t x1 = Wire.read();
        uint8_t y0 = Wire.read();
        uint8_t y1 = Wire.read();
        uint8_t z0 = Wire.read();
        uint8_t z1 = Wire.read();
        // Convert to signed 16-bit integers
        int16_t rawX = (int16_t)((x1 << 8) | x0);
        int16_t rawY = (int16_t)((y1 << 8) | y0);
        int16_t rawZ = (int16_t)((z1 << 8) | z0);
        Vector vals = Vector(rawX, rawY, rawZ);
        vals /= 256.0f;         // Scale factor for ±16g at full resolution
        return vals;
    }
    return Vector(err, err, err);
}

/****************************************************************************/
bool TiltSensor::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

/****************************************************************************/
uint8_t TiltSensor::readRegister(uint8_t reg) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR, READ_ONE_BYTE);
    return Wire.available() ? Wire.read() : 0;
}   

/****************************************************************************/
void TiltSensor::readAndPrintInterruptSource() {
    uint8_t intSrc = readRegister(INT_SRC);
    Serial.print("Interrupt Source: 0x");
    Serial.print(intSrc, HEX);
    Serial.print(" (");
    bool first = true;
    const char* sources[] = {
        "OVERRUN", "WATERMARK", "FREE_FALL", "INACTIVITY",
        "ACTIVITY", "DOUBLE_TAP", "SINGLE_TAP", "DATA_READY"
    };
    for (int i = 0; i < 8; i++) {
        if (intSrc & (1 << i)) {
            if (!first) Serial.print(" | ");
            Serial.print(sources[7-i]);
            first = false;
        }
    }
    Serial.println(")");
}

/****************************************************************************/
void TiltSensor::handleINT1() {
    if (instance) {
        instance->int1Triggered = true;
    }
}

/****************************************************************************/
void TiltSensor::handleINT2() {
    if (instance) {
        instance->int2Triggered = true;
    }   
}

/****************************************************************************/