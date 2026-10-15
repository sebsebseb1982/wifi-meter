#ifndef PINOUT_H
#define PINOUT_H

// OLED SSD1306 (I2C, default ESP32 bus)
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define OLED_I2C_ADDRESS 0x3C

// Push buttons (active HIGH, external pull-down on the PCB)
#define BUTTON_LEFT_PIN 32
#define BUTTON_MIDDLE_PIN 33
#define BUTTON_RIGHT_PIN 25

// Light sensor (photoresistor divider). GPIO 26 is on ADC2, which the WiFi driver
// shares: see light-sensor.cpp
#define LIGHT_SENSOR_PIN 26

// LED of the PCB (driven HIGH = on)
#define STATUS_LED_PIN 17

#endif
