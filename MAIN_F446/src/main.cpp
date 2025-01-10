#include <Arduino.h>
#include "./device/device.h"
#include <Wire.h>

#define I2C_SDA PB7
#define I2C_SCL PB6

HardwareSerial uart1(PA10, PA9);              // USB
HardwareSerial uart2(PA3, PA2);               // LINE BOARD
HardwareSerial uart3(PC_11_ALT1, PC_10_ALT1); // STS3032
HardwareSerial uart4(PA1, PA0);               // PWR
HardwareSerial uart6(PC7, PC6);               // OpenMV

Buzzer buzzer(PB1);
STS3032 sts3032(&uart3);
Microservo servo(&uart4);

LoadCell loadcell(PC0, PC1);
LineUnit line(&uart2);
ToF tof(PA5, PA6, PA7, PA8, PA12, PA13, PA14);
BNO055 bno(55, &Wire);

volatile bool isRescue;

extern void LineSetup();
extern void LineLoop();
extern void RescueSetup();
extern void RescueLoop();

void init_i2c();

void setup()
{

  // init UART (others are initialized in their own classes)
  uart1.begin(115200); // USB for debug
  uart4.begin(115200); // PWR send servo command and receive tof sensor data

  // init I2C sensors
  init_i2c();
  tof.init();
  bno.begin();

  sts3032.isDisabled = false;
  buzzer.isDisabled = false;
  sts3032.stop();

  LineSetup();

  buzzer.boot();
}

void loop()
{
  // line.read();
  // line.print(&uart1);
  // return;

  if (!isRescue)
  {
    LineLoop();
    if (isRescue)
    {
      RescueSetup();
    }
  }
  else
  {
    RescueLoop();
    if (!isRescue)
    {
      LineSetup();
    }
  }
}

void init_i2c()
{
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
}
