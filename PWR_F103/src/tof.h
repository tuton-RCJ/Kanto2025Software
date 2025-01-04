#ifndef tof_H
#define tof_H

#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

class ToF
{
public:
    ToF(int tofPin1, int tofPin2);
    void init();
    int tof_values[2];
    void getTofValues();

private:
    int tof_pins[2];
    VL53L0X tof_sensors[2];
    int init_tof_sensors();
};

#endif