// Solenoid.h
#ifndef SOLENOID_H
#define SOLENOID_H

#include <Arduino.h>

class Solenoid {

private:
    uint8_t _pin;
    uint8_t _chan;
    uint32_t _freq;
    uint8_t _res;
    bool _init;
    uint32_t _duty;

public:
    /**
     * @brief Constructor for Solenoid driver with IRFZ44N MOSFET
     * @param pin GPIO pin number (default: 1)
     * @param channel LEDC channel (default: 0)
     */
    Solenoid(uint8_t pin = 1, uint8_t channel = 0)
        : _pin(pin), _chan(channel), _freq(4000), _res(12), _init(false) {}

    /**
     * @brief Initialize the solenoid driver
     * @return true on success, false on failure
     */
    bool begin() {
        // Configure LEDC timer and channel
        ledcSetup(_chan, _freq, _res);
        ledcAttachPin(_pin, _chan);
        
        // Start with duty cycle 0 (off)
        ledcWrite(_chan, 0);
        
        _init = true;
        _duty = 0;
        
        Serial.printf("Solenoid initialized on Pin %d, Channel %d, 4kHz, 12-bit (4096 steps)\n", _pin, _chan);
        return true;
    }

    /**
     * @brief Set PWM duty cycle
     * @param duty Duty cycle value (0 to 4095)
     */
    void setDuty(uint32_t duty) {
        if (!_init) {
            Serial.println("Solenoid not initialized!");
            return;
        }
        
        if (duty > 4095) {
            duty = 4095;
            Serial.println("Duty cycle clamped to maximum: 100% (4095)");
        }
        
        ledcWrite(_chan, duty);
        _duty = duty;
    }

    /**
     * @brief Set PWM duty cycle as percentage
     * @param percentage Percentage value (0.0 to 100.0)
     */
    void setDutyPercentage(float percentage) {
        percentage = constrain(percentage, 0.0f, 100.0f);
        uint32_t duty = (percentage / 100.0f) * 4095.0f;
        setDuty(duty);
    }

    /**
     * @brief Turn solenoid fully on (100% duty cycle)
     */
    void on() {
        setDuty(4095); // 100% duty for 12-bit
    }

    /**
     * @brief Turn solenoid fully off (0% duty cycle)
     */
    void off() {
        setDuty(0);
    }

    /**
     * @brief Get current duty cycle value
     * @return uint32_t Current duty cycle value (0-4095)
     */
    uint32_t getDuty() const {
        return _duty;
    }

    /**
     * @brief Get current duty cycle as percentage
     * @return float Current duty cycle percentage
     */
    float getDutyPercentage() const {
        return (_duty / 4095.0f) * 100.0f;
    }

    /**
     * @brief Check if solenoid is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const {
        return _init;
    }

    /**
     * @brief Get GPIO pin number
     * @return uint8_t GPIO pin number
     */
    uint8_t getPin() const {
        return _pin;
    }

    /**
     * @brief Get PWM channel
     * @return uint8_t PWM channel
     */
    uint8_t getChannel() const {
        return _chan;
    }

    /**
     * @brief Get PWM frequency
     * @return uint32_t PWM frequency in Hz
     */
    uint32_t getFrequency() const {
        return _freq;
    }

    /**
     * @brief Get PWM resolution in bits
     * @return uint8_t PWM resolution (12 bits)
     */
    uint8_t getResolution() const {
        return _res;
    }


};

#endif // SOLENOID_H