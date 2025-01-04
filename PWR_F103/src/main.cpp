#include <Arduino.h>
#include "Servo.h"
#include "tof.h"

#define ServoHandLeft 0

#define I2C_SCL PB6 
#define I2C_SDA PB7

ToF tof(PA15, PB15);

HardwareSerial Serial1(PA10, PA9);
void init_i2c()
{
  Wire.setSCL(I2C_SCL);
  Wire.setSDA(I2C_SDA);
  Wire.begin();
}

void setup() {
  // put your setup code here, to run once:
  Serial1.begin(115200);
  init_i2c();
  tof.init();
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  tof.getTofValues();
  Serial1.print(tof.tof_values[0]);
  Serial1.print(" ");
  Serial1.println(tof.tof_values[1]);
}

