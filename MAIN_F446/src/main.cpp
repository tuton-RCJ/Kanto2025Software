#include <Arduino.h>
#include "./device/device.h"
#include <Wire.h>

#define I2C_SDA PB7
#define I2C_SCL PB6

#define StartSwitch PA4

HardwareSerial uart1(PA10, PA9);  // USB
HardwareSerial uart2(PA3, PA2);   // LINE BOARD
HardwareSerial uart3(PC11, PC10); // STS3032
HardwareSerial uart4(PA1, PA0);   // PWR
//  HardwareSerial uart5(PD2, PC12);  // not used
HardwareSerial uart6(PC7, PC6); // OpenMV

Buzzer buzzer(PB1);
LoadCell loadcell(PC0, PC1);
LineUnit line(&uart2);
STS3032 sts3032(&uart3);
ToF tof(PA5, PA6, PA7, PA8, PA12, PA13, PA14);

TwoWire Wire2(PB3, PB10);

BNO055 bno(55, &Wire);

MPU6050 mpu6050(&Wire2);

void LineTrace();
void CheckRed();
void CheckGreen();

void CheckObject();
bool TurningObject = false;
void TurnObject();

// FOR LINETRACE
int Kps[15] = {-8, -8, -6, -4, -3, -2, -2, 0, 2, 2, 3, 4, 6, 8, 8};
int threshold = 800;
int silver_threshould = 100;
int Kp = 16;
int Kd = 0;
int Ki = 0;
int lastError = 0;
int sumError = 0;
int speed = 30;
int normalSpeed = 40;

int SlopeStatus = 0; // 0:平坦 1:上り 2:下り

float heading, pitch, roll;

void setSlopeStatus();

bool isRescue = false;

void init_i2c()
{
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
  Wire2.begin();
}

void setup()
{
  uart1.begin(115200);

  uart1.println("Hello, World!");

  init_i2c();

  tof.init();

  // line.setBrightness(48);
  loadcell.init();

  // TurningObject = false;
  // pinMode(StartSwitch, INPUT);
  // buzzer.kouka();
  // sts3032.drive(40, 0);

  // センサーの初期化
  if (!bno.begin())
  {
    uart1.println("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1)
      ;
  }

  // Serial.println("success");
  pinMode(StartSwitch, INPUT);
  buzzer.boot();
  sumError = 0;
  mpu6050.init();

  isRescue=false;
}

void loop()
{
  line.read();

  if (digitalRead(StartSwitch) == HIGH)
  {
    sts3032.stop();
    return;
  }

  if (TurningObject)
  {
    tof.getTofValues();
    TurnObject();
    return;
  }



  LineTrace();
  CheckRed();
  CheckGreen();

  CheckObject();
  bno.readEulerAngles(heading, pitch, roll);
  setSlopeStatus();
}

void LineTrace()
{
  int error = 0;
  int pid = 0;

  int turnRate = 0;
  for (int i = 0; i < 15; i++)
  {
    if (line._photoReflector[i] > threshold)
    {
      error += Kps[i];
    }

    if (line._photoReflector[i] < silver_threshould)
    {
      isRescue = true;
      sts3032.stop();
      buzzer.EnterEvacuationZone();
      return;
    }
  }
  if (abs(error) > 20 && line._frontPhotoReflector > threshold)
  {
    error = 0;
    speed = 20;
  }
  else
  {
    speed = normalSpeed;
  }
  sumError += error;
  pid = Kp * error + Ki * sumError + Kd * (error - lastError);
  lastError = error;

  turnRate = pid;
  sts3032.drive(speed, turnRate);
}

void CheckRed()
{
  if (line.colorLTime[3] > 0 && millis() - line.colorLTime[3] < 100)
  {
    sts3032.stop();
    buzzer.kouka();
  }
}

void CheckGreen()
{
  if ((line._photoReflector[2] > threshold || line._photoReflector[12] > threshold) && line._frontPhotoReflector)
  {
    int p = 0;
    if (line.colorLTime[2] > 0 && millis() - line.colorLTime[2] < 400)
    {
      p += 1;
    }
    if (line.colorRTime[2] > 0 && millis() - line.colorRTime[2] < 400)
    {
      p += 2;
    }
    if (p > 0)
    {
      sts3032.stop();
      buzzer.GreenMarker(p);
      if (p == 1)
      {
        sts3032.drive(40, 0);
        delay(400);
        sts3032.turn(30, -80);

        // sts3032.drive(50, -85);
        // delay(1000);
        sts3032.stop();
      }
      if (p == 2)
      {
        sts3032.drive(40, 0);
        delay(400);
        sts3032.turn(30, 80);
        // sts3032.drive(50, 85);
        // delay(1000);
        sts3032.stop();
      }
      if (p == 3)
      {
        sts3032.turn(50, 180);
        sts3032.stop();
      }
    }
  }
}

void CheckObject()
{
  // bool flag = false;
  // for (int i = 2; i < 7; i++)
  // {
  //   if (tof.tof_values[i] < 100)
  //   {
  //     flag = true;
  //   }
  // }
  // if (flag)
  // {
  //   speed = 30;
  // }
  // else
  // {
  //   speed = 50;
  // }
  loadcell.read();
  if (loadcell.values[0] > 150 || loadcell.values[1] > 150)
  {
    sts3032.stop();
    buzzer.ObjectDetected();
    sts3032.drive(-50, 0);
    delay(500);
    sts3032.turn(50, 90);
    TurningObject = true;
  }
}

void TurnObject()
{
  if (tof.tof_values[1] < 120)
  {
    buzzer.beep(440, 0.5);
    sts3032.drive(40, 0);
  }
  else
  {
    buzzer.beep(880, 0.5);
    sts3032.drive(40, -70);
  }
  bool blackFlag = false;
  for (int i = 0; i < 15; i++)
  {
    if (line._photoReflector[i] > threshold)
    {
      blackFlag = true;
    }
  }
  if (blackFlag)
  {
    sts3032.stop();
    buzzer.ObjectDetected();
    sts3032.turn(50, 60);
    TurningObject = false;
  }
}

void setSlopeStatus()
{
  if (pitch > 15)
  {
    SlopeStatus = 1;
    buzzer.beep(392, 0.5);
  }
  else if (pitch < -15)
  {
    SlopeStatus = 2;
    buzzer.beep(262, 0.5);
  }
  else
  {
    SlopeStatus = 0;
  }
}