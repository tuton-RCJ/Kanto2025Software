#ifndef tof_H
#define tof_H

#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

class ToF
{
public:
    ToF(int front1, int front2, int front3, int front4, int front5, int side1, int side2);
    void init();
    int tof_values[7];
    void getTofValues();

private:
    int tof_pins[7];
    VL53L0X tof_sensors[7];
    int init_tof_sensors();
};

#endif