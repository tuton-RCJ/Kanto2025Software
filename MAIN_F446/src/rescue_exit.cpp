#include <Arduino.h>
#include "./device/device.h"

// communication
extern HardwareSerial uart1;
extern HardwareSerial uart4;
extern HardwareSerial uart6;

// Sensor
extern LoadCell loadcell;
extern LineUnit line;
extern ToF tof;
extern BNO055 bno;

// Actuatr
extern STS3032 sts3032;
extern Microservo servo;
extern Buzzer buzzer;

extern bool isRescue;
