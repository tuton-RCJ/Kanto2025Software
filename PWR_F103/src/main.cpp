#include <Arduino.h>
#include "Servo.h"
#include "tof.h"

#define ServoHandLeft PA1
#define ServoHandRight PA0
#define ServoArmLeft PA3
#define ServoArmRight PA2
#define ServoBasketLeft PA7
#define ServoBasketRight PA6

#define I2C_SCL PB6
#define I2C_SDA PB7

ToF tof(PA15, PB15);

HardwareSerial Serial1(PA10, PA9);
HardwareSerial uart3(PB11, PB10);

Servo servoHandLeft;
Servo servoHandRight;
Servo servoArmLeft;
Servo servoArmRight;
Servo servoBasketLeft;
Servo servoBasketRight;

void HandClose()
{
  servoHandLeft.write(10);
  servoHandRight.write(170);
}
void HandOpen()
{
  servoHandLeft.write(120);
  servoHandRight.write(60);
}
void ArmUp()
{
  servoArmLeft.write(30);
  servoArmRight.write(100);
}

void ArmDown()
{
  servoArmLeft.write(90);
  servoArmRight.write(50);
  HandOpen();
  delay(300);
  servoArmLeft.write(150);
  servoArmRight.write(0);
}

void BasketClose()
{
  servoBasketLeft.write(110);
  servoBasketRight.write(60);
}
void BasketOpen()
{
  servoBasketLeft.write(70);
  servoBasketRight.write(100);
}
void init_i2c()
{
  Wire.setSCL(I2C_SCL);
  Wire.setSDA(I2C_SDA);
  Wire.begin();
}

void AttachServo()
{
  servoHandLeft.attach(ServoHandLeft, 500, 2500);
  servoHandRight.attach(ServoHandRight, 500, 2500);
  servoArmLeft.attach(ServoArmLeft, 500, 2500);
  servoArmRight.attach(ServoArmRight, 470, 2500);

  servoBasketLeft.attach(ServoBasketLeft, 500, 2500);
  servoBasketRight.attach(ServoBasketRight, 500, 2500);
}

void DetachServo()
{
  servoHandLeft.detach();
  servoHandRight.detach();
  servoArmLeft.detach();
  servoArmRight.detach();
  servoBasketLeft.detach();
  servoBasketRight.detach();
}

void setup()
{
  // put your setup code here, to run once:
  Serial1.begin(115200);
  init_i2c();
  tof.init();
  delay(100);

  AttachServo();
  BasketClose();
  HandClose();
  ArmUp();
  delay(1000);
  DetachServo();
}

void loop()
{
  // put your main code here, to run repeatedly:
  tof.getTofValues();
  // Serial1.print(tof.tof_values[0]);
  // Serial1.print(" ");
  // Serial1.println(tof.tof_values[1]);

  uart3.print(tof.tof_values[0]);
  uart3.print(" ");
  uart3.println(tof.tof_values[1]);

  if(uart3.available()){
    String data = uart3.readString();
    if(data=="HandClose"){
      HandClose();
    }
    if(data=="HandOpen"){
      HandOpen();
    }
    if(data=="ArmUp"){
      ArmUp();
    }
    if(data=="ArmDown"){
      ArmDown();
    }
    if(data=="BasketClose"){
      BasketClose();
    }
    if(data=="BasketOpen"){
      BasketOpen();
    }
    if(data=="AttachServo"){
      AttachServo();
    }
    if(data=="DetachServo"){
      DetachServo();
    }
  }
  // ArmDown();
  // delay(1000);
  // HandClose();
  // delay(1000);
  // ArmUp();
  // delay(1000);
  // BasketOpen();
  // delay(1000);
  // BasketClose();
}
