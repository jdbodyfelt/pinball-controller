#ifdef USB_GAMEPAD
    #include <USBHIDGamepad.h>
    USBHIDGamepad usbGamepad;
#endif

#ifdef BLE_GAMEPAD  
    #include <BleGamepad.h>
    BleGamepad bleGamepad("Pinball Controller");
#endif