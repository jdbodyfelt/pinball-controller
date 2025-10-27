#include <Wire.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define ADXL345_ADDRESS 0x53

bool deviceConnected = false;
BLEServer *pServer;
BLECharacteristic *pReportCharacteristic;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    }

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        BLEDevice::startAdvertising();
    }
};

static const uint8_t hidReportDescriptor[] = {
    0x05, 0x01, 0x09, 0x04, 0xA1, 0x01, 0x15, 0x00,
    0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x03, 0x09,
    0x30, 0x09, 0x31, 0x09, 0x32, 0x81, 0x02, 0xC0};

void setup()
{
    Wire.begin(8, 9);

    // Initialize ADXL345
    Wire.beginTransmission(ADXL345_ADDRESS);
    Wire.write(0x2D);
    Wire.write(0x08);
    Wire.endTransmission();

    Wire.beginTransmission(ADXL345_ADDRESS);
    Wire.write(0x31);
    Wire.write(0x01);
    Wire.endTransmission();
    // Create HID service
    BLEService *pHIDService = pServer->createService(BLEUUID((uint16_t)0x1812));

    // Protocol Mode Characteristic (required)
    BLECharacteristic *pProtocolMode = pHIDService->createCharacteristic(
        BLEUUID((uint16_t)0x2A4E),
        BLECharacteristic::PROPERTY_READ);
    uint8_t protocolMode[] = {0x01}; // Report protocol
    pProtocolMode->setValue(protocolMode, 1);

    // Report Map Characteristic
    BLECharacteristic *pReportMap = pHIDService->createCharacteristic(
        BLEUUID((uint16_t)0x2A4B),
        BLECharacteristic::PROPERTY_READ);
    pReportMap->setValue((uint8_t *)hidReportDescriptor, sizeof(hidReportDescriptor));

    // HID Information Characteristic (required)
    BLECharacteristic *pHIDInfo = pHIDService->createCharacteristic(
        BLEUUID((uint16_t)0x2A4A),
        BLECharacteristic::PROPERTY_READ);
    uint8_t hidInfo[] = {0x01, 0x01, 0x00, 0x02}; // bcdHID 1.1, bCountryCode 0, Flags: RemoteWake, NormallyConnectable
    pHIDInfo->setValue(hidInfo, 4);

    // Report Characteristic (for sending data)
    pReportCharacteristic = pHIDService->createCharacteristic(
        BLEUUID((uint16_t)0x2A4D),
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);

    // Start services
    pHIDService->start();

    // Advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLEUUID((uint16_t)0x1812));
    pAdvertising->setAppearance(0x03C4); // Joystick appearance
    pAdvertising->start();
}

void loop()
{
    if (deviceConnected)
    {
        int16_t x, y, z;

        Wire.beginTransmission(ADXL345_ADDRESS);
        Wire.write(0x32);
        Wire.endTransmission(false);
        Wire.requestFrom(ADXL345_ADDRESS, 6, 1);

        if (Wire.available() == 6)
        {
            x = (Wire.read() | (Wire.read() << 8));
            y = (Wire.read() | (Wire.read() << 8));
            z = (Wire.read() | (Wire.read() << 8));

            uint8_t joyX = map(constrain(x, -256, 255), -256, 255, 0, 255);
            uint8_t joyY = map(constrain(y, -256, 255), -256, 255, 0, 255);
            uint8_t joyZ = map(constrain(z, -256, 255), -256, 255, 0, 255);

            uint8_t joystickData[3] = {joyX, joyY, joyZ};
            pReportCharacteristic->setValue(joystickData, 3);
            pReportCharacteristic->notify();
        }
    }
    delay(20);
}