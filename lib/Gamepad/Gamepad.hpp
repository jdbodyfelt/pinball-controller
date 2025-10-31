#ifdef USB_GAMEPAD
    #include <USBHIDGamepad.h>
    USBHIDGamepad usbGamepad;
#endif

#ifdef BLE_GAMEPAD  
    #include <BleGamepad.h>
    BleGamepad bleGamepad("Pinball Controller");
#endif

class Gamepad {
protected:
    Accelerometer accel;
    ButtonManager buttons;
    Joystick joystick;
    uint32_t lastSampleTime;
    
public:
    virtual void update() = 0;
    virtual GamepadState getState() = 0;
};

class MotionGamepad : public Gamepad {
private:
    struct Sample {
        int16_t x, y, z;
        uint32_t timestamp;
    };
    
    CircularBuffer<Sample, 64> sampleBuffer;
    
public:
    void update() override {
        // Strategy depends on your timing approach
        accel.update();
        buttons.update();
        joystick.update();
    }
    
    GamepadState getState() override {
        GamepadState state;
        state.acceleration = accel.getLatest();
        state.buttons = buttons.getState();
        state.timestamp = micros();
        return state;
    }
};