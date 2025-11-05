#pragma once
#include <Arduino.h>
#include "LED.hpp"
#include "Switch.hpp"

/******************************************************************************/
class ArcadeButton
{
private:
    CallbackSwitch button;
    PulseLED redLED;
    PulseLED greenLED;
    PulseLED blueLED;

public:
    enum LEDMode
    {
        OFF,
        SOLID_RED,
        SOLID_GREEN,
        SOLID_BLUE,
        PULSE_RED,
        PULSE_GREEN,
        PULSE_BLUE,
        PRESS_COLOR_CYCLE,
        PRESS_RAINBOW,
        STATUS_INDICATOR,
        BREATHING_RAINBOW,
        CHASE,
        RANDOM_COLOR_PRESS,
        MODE_COUNT
    };

    LEDMode currentMode;
    int colorCycleIndex;

    // Constructor with separate pins for each color
    ArcadeButton(int switchPin, int redPin, int greenPin, int bluePin,
                 unsigned long debounceTime = 50,
                 unsigned long pulseOnTime = 500,
                 unsigned long pulseOffTime = 500)
        : button(switchPin, debounceTime),
          redLED(redPin, pulseOnTime, pulseOffTime),
          greenLED(greenPin, pulseOnTime, pulseOffTime),
          blueLED(bluePin, pulseOnTime, pulseOffTime),
          currentMode(PULSE_BLUE),
          colorCycleIndex(0)
    {

        allLEDsOff();
        setupModeCallbacks();
    }

private:
    void allLEDsOff()
    {
        redLED.off();
        greenLED.off();
        blueLED.off();
    }

    void setSolidColor(bool red, bool green, bool blue)
    {
        redLED.stopPulse();
        greenLED.stopPulse();
        blueLED.stopPulse();

        if (red)
            redLED.on();
        else
            redLED.off();
        if (green)
            greenLED.on();
        else
            greenLED.off();
        if (blue)
            blueLED.on();
        else
            blueLED.off();
    }

    void setPulseColor(bool red, bool green, bool blue)
    {
        // Use current timing for each LED
        if (red)
        {
            redLED.startPulse(redLED.getOnTime(), redLED.getOffTime());
        }
        else
        {
            redLED.off();
        }
        if (green)
        {
            greenLED.startPulse(greenLED.getOnTime(), greenLED.getOffTime());
        }
        else
        {
            greenLED.off();
        }
        if (blue)
        {
            blueLED.startPulse(blueLED.getOnTime(), blueLED.getOffTime());
        }
        else
        {
            blueLED.off();
        }
    }

    void cycleToNextColor()
    {
        colorCycleIndex = (colorCycleIndex + 1) % 7;
        switch (colorCycleIndex)
        {
        case 0:
            setSolidColor(0, 0, 0);
            break; // Off
        case 1:
            setSolidColor(1, 0, 0);
            break; // Red
        case 2:
            setSolidColor(0, 1, 0);
            break; // Green
        case 3:
            setSolidColor(0, 0, 1);
            break; // Blue
        case 4:
            setSolidColor(1, 1, 0);
            break; // Yellow
        case 5:
            setSolidColor(1, 0, 1);
            break; // Magenta
        case 6:
            setSolidColor(0, 1, 1);
            break; // Cyan
        }
    }

    void setRainbowColor(int step)
    {
        switch (step % 6)
        {
        case 0:
            setSolidColor(1, 0, 0);
            break; // Red
        case 1:
            setSolidColor(1, 1, 0);
            break; // Yellow
        case 2:
            setSolidColor(0, 1, 0);
            break; // Green
        case 3:
            setSolidColor(0, 1, 1);
            break; // Cyan
        case 4:
            setSolidColor(0, 0, 1);
            break; // Blue
        case 5:
            setSolidColor(1, 0, 1);
            break; // Magenta
        }
    }

    void setRandomColor()
    {
        int color = random(7);
        switch (color)
        {
        case 0:
            setSolidColor(1, 0, 0);
            break; // Red
        case 1:
            setSolidColor(0, 1, 0);
            break; // Green
        case 2:
            setSolidColor(0, 0, 1);
            break; // Blue
        case 3:
            setSolidColor(1, 1, 0);
            break; // Yellow
        case 4:
            setSolidColor(1, 0, 1);
            break; // Magenta
        case 5:
            setSolidColor(0, 1, 1);
            break; // Cyan
        case 6:
            setSolidColor(1, 1, 1);
            break; // White
        }
    }

public:
    void setupModeCallbacks()
    {
        // Store the existing callbacks before clearing
        auto existingPressCallback = button.getPressCallback();
        auto existingReleaseCallback = button.getReleaseCallback();

        // Clear callbacks temporarily
        button.onPress(nullptr);
        button.onRelease(nullptr);

        allLEDsOff();

        // Set up mode-specific behavior
        switch (currentMode)
        {
        case OFF:
            setSolidColor(0, 0, 0);
            break;
        case SOLID_RED:
            setSolidColor(1, 0, 0);
            break;
        case SOLID_GREEN:
            setSolidColor(0, 1, 0);
            break;
        case SOLID_BLUE:
            setSolidColor(0, 0, 1);
            break;
        case PULSE_RED:
            setPulseColor(1, 0, 0);
            break;
        case PULSE_GREEN:
            setPulseColor(0, 1, 0);
            break;
        case PULSE_BLUE:
            setPulseColor(0, 0, 1);
            break;
        case PRESS_COLOR_CYCLE:
            setSolidColor(0, 0, 0);
            button.onPress([this]()
                           { cycleToNextColor(); });
            break;
        case PRESS_RAINBOW:
            setSolidColor(1, 0, 0);
            colorCycleIndex = 0;
            button.onPress([this]()
                           {
        colorCycleIndex = (colorCycleIndex + 1) % 6;
        setRainbowColor(colorCycleIndex); });
            break;
        case STATUS_INDICATOR:
            setSolidColor(0, 1, 0);
            button.onPress([this]()
                           { setSolidColor(1, 0, 0); });
            button.onRelease([this]()
                             { setSolidColor(0, 1, 0); });
            break;
        case BREATHING_RAINBOW:
            colorCycleIndex = 0;
            setRainbowColor(0);
            break;
        case CHASE:
            setSolidColor(1, 0, 0);
            break;
        case RANDOM_COLOR_PRESS:
            setSolidColor(0, 0, 0);
            button.onPress([this]()
                           { setRandomColor(); });
            break;
        }

        // RESTORE the external callbacks by chaining them
        if (existingPressCallback)
        {
            auto currentPressCallback = button.getPressCallback();
            if (currentPressCallback)
            {
                // Chain the callbacks: mode callback first, then external callback
                button.onPress([currentPressCallback, existingPressCallback]()
                               {
        currentPressCallback();
        existingPressCallback(); });
            }
            else
            {
                // No mode callback, just use external callback
                button.onPress(existingPressCallback);
            }
        }

        if (existingReleaseCallback)
        {
            auto currentReleaseCallback = button.getReleaseCallback();
            if (currentReleaseCallback)
            {
                // Chain the callbacks: mode callback first, then external callback
                button.onRelease([currentReleaseCallback, existingReleaseCallback]()
                                 {
        currentReleaseCallback();
        existingReleaseCallback(); });
            }
            else
            {
                // No mode callback, just use external callback
                button.onRelease(existingReleaseCallback);
            }
        }
    }

    void update()
    {
        button.update();
        redLED.update();
        greenLED.update();
        blueLED.update();
        updateAutomaticModes();
    }

private:
    unsigned long lastAutoUpdate = 0;
    const unsigned long AUTO_UPDATE_INTERVAL = 1000;

    void updateAutomaticModes()
    {
        unsigned long currentMillis = millis();
        if (currentMillis - lastAutoUpdate >= AUTO_UPDATE_INTERVAL)
        {
            lastAutoUpdate = currentMillis;

            switch (currentMode)
            {
            case BREATHING_RAINBOW:
                colorCycleIndex = (colorCycleIndex + 1) % 6;
                setRainbowColor(colorCycleIndex);
                break;

            case CHASE:
                colorCycleIndex = (colorCycleIndex + 1) % 3;
                switch (colorCycleIndex)
                {
                case 0:
                    setSolidColor(1, 0, 0);
                    break;
                case 1:
                    setSolidColor(0, 1, 0);
                    break;
                case 2:
                    setSolidColor(0, 0, 1);
                    break;
                }
                break;
            }
        }
    }

public:
    void setMode(LEDMode mode)
    {
        currentMode = mode;
        colorCycleIndex = 0;
        setupModeCallbacks();
    }

    void nextMode()
    {
        int nextMode = (currentMode + 1) % MODE_COUNT;
        setMode(static_cast<LEDMode>(nextMode));
    }

    String getModeName()
    {
        switch (currentMode)
        {
        case OFF:
            return "OFF";
        case SOLID_RED:
            return "SOLID_RED";
        case SOLID_GREEN:
            return "SOLID_GREEN";
        case SOLID_BLUE:
            return "SOLID_BLUE";
        case PULSE_RED:
            return "PULSE_RED";
        case PULSE_GREEN:
            return "PULSE_GREEN";
        case PULSE_BLUE:
            return "PULSE_BLUE";
        case PRESS_COLOR_CYCLE:
            return "PRESS_COLOR_CYCLE";
        case PRESS_RAINBOW:
            return "PRESS_RAINBOW";
        case STATUS_INDICATOR:
            return "STATUS_INDICATOR";
        case BREATHING_RAINBOW:
            return "BREATHING_RAINBOW";
        case CHASE:
            return "CHASE";
        case RANDOM_COLOR_PRESS:
            return "RANDOM_COLOR_PRESS";
        default:
            return "UNKNOWN";
        }
    }

    bool isPressed() { return button.isPressed(); }
    bool wasPressed() { return button.wasPressed(); }
    bool wasReleased() { return button.wasReleased(); }

    void setColor(bool red, bool green, bool blue)
    {
        setSolidColor(red, green, blue);
    }

    void setPulse(bool red, bool green, bool blue,
                  unsigned long onTime = 500, unsigned long offTime = 500)
    {
        redLED.setPulseTiming(onTime, offTime);
        greenLED.setPulseTiming(onTime, offTime);
        blueLED.setPulseTiming(onTime, offTime);
        setPulseColor(red, green, blue);
    }

    void allOff()
    {
        allLEDsOff();
    }

    void onPress(void (*callback)())
    {
        button.onPress(callback);
    }

    void onRelease(void (*callback)())
    {
        button.onRelease(callback);
    }

    PulseLED &getRedLED() { return redLED; }
    PulseLED &getGreenLED() { return greenLED; }
    PulseLED &getBlueLED() { return blueLED; }
    CallbackSwitch &getSwitch() { return button; }
};
/******************************************************************************/