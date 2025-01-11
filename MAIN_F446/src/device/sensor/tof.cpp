#include "tof.h"

ToF::ToF(int front1, int front2, int front3, int front4, int front5, int side1, int side2)
{
    tof_pins[0] = front1;
    tof_pins[1] = front2;
    tof_pins[2] = front3;
    tof_pins[3] = front4;
    tof_pins[4] = front5;
    tof_pins[5] = side1;
    tof_pins[6] = side2;
}

void ToF::init()
{
    int e = init_tof_sensors();
}

void ToF::getTofValues()
{
    for (int i = 0; i < 7; i++)
    {
        tof_values[i] = tof_sensors[i].readRangeContinuousMillimeters();
    }
}

int ToF::init_tof_sensors()
{
    for (int i = 0; i < 7; i++)
    {
        pinMode(tof_pins[i], OUTPUT);
        digitalWrite(tof_pins[i], LOW);
    }
    delay(100);
    for (int i = 0; i < 7; i++)
    {

        digitalWrite(tof_pins[i], HIGH);
        delay(20);
        tof_sensors[i].setTimeout(500);
        if (!tof_sensors[i].init())
        {
            // debugSerial->println("Failed to detect and initialize sensor!");
            return 1;
        }
        tof_sensors[i].setAddress(0x30 + i);

        tof_sensors[i].startContinuous(0);
    }
    return 0;
}

void ToF::print(HardwareSerial *serial)
{
    serial->print("R: ");
    serial->print(tof_values[0]);
    serial->print(" L: ");
    serial->print(tof_values[1]);
    serial->print(" F: ");
    for (int i = 2; i < 7; i++)
    {
        serial->print(tof_values[i]);
        serial->print(" ");
    }
    serial->println();
}