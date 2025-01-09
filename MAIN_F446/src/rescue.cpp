#include <Arduino.h>
#include "./device/device.h"
#define PI 3.14159265358979323846

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

//-----------------------------------------

// todo tof の値は適当なのでものけんで調節する
//      ブザー音を鳴らすようにする

int SaveVictimCount = 0; // 生存者の救出数
int DeadVictimCount = 0; // 死亡者の救出(?)数
int EntrancePositon = 2; // 入口が右側にあるかどうか 0: 右側, 1: 左側, 2: 未確定

int NowAngle;
int TargetX;         // 目標の x 座標
int midX = 16;       // カメラの中央の x 座標
bool InEntrance;     // 入口にいるかどうか
bool VictimDetected; // 被災者が検出されたかどうか
bool ZoneDetected;   // 避難ゾーンが検出されたかどうか
bool NearbyVictim;   // 被災者の目の前にいるか
bool HaveVictim;     // 被災者を持っているかどうか
int HFOV = 60;       // OpenMV の水平視野角

int PGain = 5; // カメラから送られてきた重心を基準に前に進む時のP 制御のゲイン

int SaveVictimXY[2]; // OpenMV から送られてきた生存者情報
int DeadVictimXY[2]; // OpenMV から送られてきた死亡者情報
int SaveVictimZone[2];
int DeadVictimZone[2];

void BallDrop();

bool GetVictimData(int flag);

void Pcontrol(int x);

void GoRandomPosition();

void GoNextDetection();

int XtoTurnRate(int x);

bool GetFrontObject();

void RescueSetup()
{
    VictimDetected = false;
    ZoneDetected = false;
    NearbyVictim = false;
    HaveVictim = false;
    InEntrance = true;
    NowAngle = 0;
    uart6.begin(115200);
    uart4.print("LightOff");
}

void RescueLoop()
{

    // 生存者の救出
    if (SaveVictimCount < 2)
    {
        // 生存者を回収済みでゾーン未検出
        if (HaveVictim && !ZoneDetected)
        {
            // 　最も大きい角度のゾーンを検出
            int MaxI = -1;
            int MaxX = 0;
            int MaxW = 0;

            int turnRate = 45;

            for (int i = 0; i < 360 / turnRate; i++)
            {
                if (GetVictimData(2))
                {
                    buzzer.DetectedGreenCorner();
                    if (SaveVictimZone[1] > MaxW)
                    {
                        MaxI = i;
                        MaxX = SaveVictimZone[0];
                        MaxW = SaveVictimZone[1];
                    }
                }
                sts3032.turn(50, turnRate);
            }

            if (MaxI != -1)
            {
                sts3032.turn(50, MaxI * turnRate + XtoTurnRate(MaxX));
                ZoneDetected = true;
                buzzer.DetectedGreenCorner();
                delay(1000);
            }
            else
            {
                buzzer.NotFound();
            }
            return;
        }
        // 生存者を回収済みでゾーンも見つけた
        else if (HaveVictim && ZoneDetected)
        {
            tof.getTofValues();
            if (GetFrontObject()) // 正面にいる。
            {
                sts3032.stop();
                sts3032.turn(50, 180);
                sts3032.drive(-30, 0);
                delay(2000);
                sts3032.stop();
                BallDrop();
                HaveVictim = false;
                ZoneDetected = false;
                SaveVictimCount++;
                sts3032.drive(50, 0);
                delay(200);
                sts3032.stop();

                return;
            }
            if (GetVictimData(2)) // 重心についてP制御
            {
                Pcontrol(SaveVictimZone[0]);
            }
        }
        // 生存者を発見していない
        else if (!VictimDetected)
        {
            if (GetVictimData(0))
            {
                buzzer.DetectedSilverBall();
                TargetX = SaveVictimXY[0];
                VictimDetected = true;
                sts3032.turn(50, XtoTurnRate(TargetX));
                servo.AttachServo();
                servo.ArmDown();
                delay(1000);
                buzzer.DetectedSilverBall();
            }
            else
            {
                buzzer.NotFound();
                sts3032.turn(50, 30);
            }
        }

        // 生存者を発見しているが未回収
        else
        {
            if (!NearbyVictim)
            {
                sts3032.drive(20, 0); // これで精度出なければ P 制御か何かする
                tof.getTofValues();
                if (GetFrontObject())
                {
                    sts3032.stop();
                    buzzer.DetectedSilverBall();
                    NearbyVictim = true;
                }
            }
            else
            {
                servo.HandClose();
                delay(1000);
                servo.ArmUp();
                delay(1000);
                servo.DetachServo();
                HaveVictim = true;
                NearbyVictim = false;
                VictimDetected = false;
            }
        }
    }

    // else if (DeadVictimCount < 1)
    // {
    //     if (HaveVictim && !ZoneDetected)
    //     {
    //         int MaxI = -1;
    //         int MaxX = 0;
    //         int MaxArea = 0;
    //         for (int i = 0; i < 8; i++)
    //         {
    //             GetVictimData(3);
    //             if (get<2>(SaveVictimZone) > MaxArea)
    //             {
    //                 MaxI = i;
    //                 MaxX = get<0>(SaveVictimZone);
    //                 MaxArea = get<2>(SaveVictimZone);
    //             }
    //         }
    //         if (MaxI != -1)
    //         {
    //             TargetX = get<0>(SaveVictimZone);
    //             sts3032.turn(50, MaxI * 45 + (MaxX - 12) * HFOV);
    //             DetectedZone = true;
    //         }
    //         else
    //         {
    //             GoNextDetection();
    //         }
    //         return;
    //     }
    //     else if (HaveVictim && ZoneDetected)
    //     {
    //         sts3032.drive(50, 0);
    //         tof.getTofValues();
    //         if (tof.tof_values[2] < 100)
    //         {
    //             sts3032.stop();
    //             sts3032.turn(50, 180);
    //             servo.BasketOpen();
    //             delay(1000);
    //             servo.BasketClose();
    //             HaveVictim = false;
    //             ZoneDetected = false;
    //             DeadVictimCount++;
    //         }
    //         return;
    //     }
    //     if (!VictimDetected)
    //     {
    //         int cnt = 0;
    //         vector<int> DeadSaveVictimXList;
    //         for (int i = 0; i < 10; i++)
    //         {
    //             GetVictimData(1);
    //             if (DeadVictimList.size() != 0)
    //             {
    //                 TempDeadVictimXList.push_back(DeadVictimList[DeadVictimList.size() / 2].first);
    //             }
    //             cnt += DeadVictimList.size();
    //         }
    //         if (cnt < VictimThreshold)
    //         {
    //             VictimDetected = false;
    //             GoNextDetection();
    //         }
    //         else
    //         {
    //             VictimDetected = true;
    //             TargetX = TempDeadVictimXList[TempSaveVictimXList.size() / 2];
    //             sts3032.turn(50, (TargetX - 12) * HFOV);
    //         }
    //     }
    //     else
    //     {
    //         if (!NearbyVictim)
    //         {
    //             sts3032.drive(40, 0);
    //             tof.getTofValues();

    //             if (tof.tof_values[2] < 60)
    //             {
    //                 sts3032.stop();
    //                 buzzer.DetectedSilverBall();
    //                 NearbyVictim = true;
    //             }
    //         }
    //         else
    //         {
    //             servo.HandClose();
    //             servo.ArmUp();
    //             DeadVictimCount++;
    //             HaveVictim = true;
    //             NearbyVictim = false;
    //             VictimDetected = false;
    //         }
    //     }
    // }
}

void BallDrop()
{
    servo.AttachServo();
    servo.BasketOpen();
    delay(2000);
    servo.BasketClose();
    delay(500);
    servo.DetachServo();
}

// uart6 には 1 or 0、 x座標 が送られる 避難ゾーンの場合は重心のX座標と幅が送られてくる
bool GetVictimData(int flag) // flag = 0: 生存者, = 1: 死亡者, 2: 生存者避難ゾーン, 3: 死亡者避難ゾーン
{
    uart6.write(flag);
    uart6.flush();
    if (flag == 0) // Silver
    {
        while (uart6.available() < 1)
            ;

        int IsValid = uart6.read();
        if (IsValid == 0)
        {
            return false;
        }
        while (uart6.available() < 1)
            ;
        int x = uart6.read();
        SaveVictimXY[0] = x;
        return true;
    }
    if (flag == 1) // Black
    {
        while (uart6.available() < 1)
            ;

        int IsValid = uart6.read();
        if (IsValid == 0)
        {
            return false;
        }
        while (uart6.available() < 1)
            ;
        int x = uart6.read();
        DeadVictimXY[0] = x;
        return true;
    }
    if (flag == 2) // Green
    {
        while (uart6.available() < 1)
            ;

        int IsValid = uart6.read();
        if (IsValid == 0)
        {
            return false;
        }
        while (uart6.available() < 2)
            ;
        int x = uart6.read();
        int w = uart6.read();
        SaveVictimZone[0] = x;
        SaveVictimZone[1] = w;
    }
    if (flag == 3) // Red
    {
        while (uart6.available() < 1)
            ;

        int IsValid = uart6.read();
        if (IsValid == 0)
        {
            return false;
        }
        while (uart6.available() < 2)
            ;
        int x = uart6.read();
        int w = uart6.read();
        DeadVictimZone[0] = x;
        DeadVictimZone[1] = w;
    }
    return true;
}

int XtoTurnRate(int x)
{
    return (HFOV * (midX - x) / (2 * midX));
}

void Pcontrol(int x)
{
    sts3032.drive(20, (midX - x) * PGain);
}

// ランダムな位置に移動する
void GoRandomPosition()
{
    tof.getTofValues();
    while (tof.tof_values[2] < 500)
    {
        int angle = random(90, 270);
        sts3032.turn(50, angle);
        tof.getTofValues();
    }
    while (tof.tof_values[2] > 500)
    {
        sts3032.drive(50, 0);
        tof.getTofValues();
    }
}

void GoNextDetection()
{

    if (InEntrance)
    {
        if (NowAngle < 90)
        {
            if (EntrancePositon == 0)
            {
                sts3032.turn(50, -45);
            }
            else if (EntrancePositon == 1)
            {
                sts3032.turn(50, 45);
            }
            else if (EntrancePositon == 2)
            {
                tof.getTofValues();
                if (tof.tof_values[5] < 400)
                { // 壁方向に回転することを防ぐ　
                    sts3032.turn(50, -135);
                    EntrancePositon = 0;
                }
                else
                {
                    sts3032.turn(50, -45);
                    EntrancePositon = 1;
                }
            }
            NowAngle += 45;
            return;
        }
        else
        {
            InEntrance = false;
            GoRandomPosition();
        }
    }
    else if (NowAngle < 360)
    {
        sts3032.turn(50, 45);
        NowAngle += 45;
    }
    else
    {
        GoRandomPosition();
        NowAngle = 0;
    }
}

bool GetFrontObject()
{
    int frontthreshold = 100;
    tof.getTofValues();
    for (int i = 2; i < 7; i++)
    {
        if (tof.tof_values[i] < frontthreshold)
        {
            return true;
        }
    }
    return false;
}