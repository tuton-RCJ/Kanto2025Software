#include <Arduino.h>    
#include "./device/device.h"


// communication 
extern HardwareSerial uart1;


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

//-----------------------------------------




void RescueSetup()
{


}


void RescueLoop()
{

}