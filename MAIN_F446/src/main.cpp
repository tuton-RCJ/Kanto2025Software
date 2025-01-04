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

BNO055 bno(55, &Wire);

void LineTrace();
void CheckRed();
void CheckGreen();

void CheckObject();
bool TurningObject = false;
void TurnObject();

// FOR LINETRACE
int Kps[15] = {-4, -4, -3, -3, -3, -2, -2, 0, 2, 2, 3, 3, 3, 4, 4};
int threshold = 800;
int Kp = 12;
int Kd = 0;
int Ki = 0;
int lastError = 0;
int sumError = 0;
int speed = 30;

void init_i2c()
{
  Wire.setSCL(I2C_SCL);
  Wire.setSDA(I2C_SDA);
  Wire.begin();
}

void setup()
{
  uart1.begin(115200);

  uart1.println("Hello, World!");

  init_i2c();

  tof.init();

  //line.setBrightness(48);
  loadcell.init();

  // TurningObject = false;
  // pinMode(StartSwitch, INPUT);
  // buzzer.kouka();
  // sts3032.drive(40, 0);

  // // センサーの初期化
  // if (!bno.begin())
  // {
  //   uart1.println("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
  //   while (1)
  //     ;
  // }

  // Serial.println("success");
  pinMode(StartSwitch, INPUT);
  buzzer.boot();
}

void loop()
{
  
  // tof.getTofValues();
  // for(int i = 0; i < 7; i++){
  //   uart1.print(tof.tof_values[i]);
  //   uart1.print(" ");
  // } 
  // uart1.println();
  // return;
  if (digitalRead(StartSwitch) == HIGH)
  {
    sts3032.stop();
    uart1.println("stop");
    return;
  }


  line.read();

  

  loadcell.read();
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
  }
  if (abs(error) > 10 && line._frontPhotoReflector > threshold)
  {
    error = 0;
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
  if (millis() - line.colorLTime[0] < 10 || millis() - line.colorRTime[0] < 10 && line._frontPhotoReflector)
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
  bool flag = false;
  for (int i = 2; i < 7; i++)
  {
    if (tof.tof_values[i] < 100)
    {
      flag = true;
    }
  }
  if (flag)
  {
    speed = 30;
  }
  else
  {
    speed = 30;
  }
  if (loadcell.values[0] > 600 || loadcell.values[1] > 600)
  {
    sts3032.stop();
    buzzer.ObjectDetected();
    sts3032.drive(-speed, 0);
    delay(200);
    sts3032.turn(50, 90);
    TurningObject = true;
  }
}

void TurnObject()
{
  if (tof.tof_values[1] < 160)
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
