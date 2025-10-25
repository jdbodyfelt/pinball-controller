# Pinball Controller: Tilt Sensor Module

This module provides support for the ADXL345 accelerometer to detect tilt events in a pinball machine. It uses I2C communication to interface with the sensor. Note that we are using the Wire library for I2C communication. The ADXL345 is a 3-axis accelerometer that can measure acceleration in the X, Y, and Z directions. This acceleration and its changes can be used to detect tilt, bump, and shock events - similar to the traditional pinball machine's "plumb bob" tilt mechanism.

The acclerometer interfaces to a ESP32-S3 N16R8 board for detection and processing of tilt events, as
well as to communicate these back to the pinball software running on a host PC.

The ADXL345 is often sold on breakout boards that include voltage regulation and level shifting, making it easy to interface with 3.3V and 5V systems. This module assumes you are using such a breakout board, configured in an eight-pin SIP format - for example, SEN-09836 from SparkFun. 

## Wiring
Connect the ADXL345 to the ESP32-S3 as follows: 
| ADXL345 | ESP32-S3 |
|---------|----------|
| GND (P1)   | GND (P22,P44) |
| VCC (P2)   | 3.3V (P1) |
| CS (P3)    | 3.3V (P1)* |
| INT1 (P4)  | GPIO (e.g., P41) |
| INT2 (P5)  | GPIO (e.g., P40) |
| SDO (P6) | GND (P22,P44)* |
| SDA (P7)   | SDA (P12) |
| SCL (P8)   | SCL (P15) |
|-------------|----------|
* CS tied to VCC to select I2C mode (SPI mode requires CS to be connected to a GPIO pin).
* SDO tied to GND to set I2C address to 0x53. If tied to VCC, address is 0x1E.

Refer to the ![ESP32-S3 datasheet](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html) and ![ADXL345 datasheet](https://cdn.sparkfun.com/assets/9/1/8/9/9/ADXL345.pdf) for more details on pin functions and configurations.

