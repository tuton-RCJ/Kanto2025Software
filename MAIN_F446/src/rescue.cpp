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
int VictimThreshold = 8; // ボールが存在することを判断する閾値
int EntrancePositon = 2; // 入口が右側にあるかどうか 0: 右側, 1: 左側, 2: 未確定
int NowAngle; 
int TargetX; // 目標の x 座標
bool InEntrance; // 入口にいるかどうか
bool VictimDetected; // 被災者が検出されたかどうか
bool ZoneDetected; // 避難ゾーンが検出されたかどうか
bool NearbyVictim; // 被災者の目の前にいるか
bool HaveVictim; // 被災者を持っているかどうか
int HFOV = 56.6; // OpenMV の水平視野角

int SaveVictimXY[2]; // OpenMV から送られてきた生存者のうち、x 座標が最も小さいものの座標
int DeadVictimXY[2]; // OpenMV から送られてきた死亡者のうち、y 座標が最も小さいものの座標
int SaveVictimZone[3];
int DeadVictimZone[3];

// uart6 には データ長 -> x座標 -> y座標 が x 座標順にそのまま結合されて送られてくる 避難ゾーンの場合は重心の座標(x,y)と面積を返す
bool GetVictimData(int flag) // flag = 0: 生存者, = 1: 死亡者, 2: 生存者避難ゾーン, 3: 死亡者避難ゾーン
{
    uart6.write(flag);
    uart6.flush();
    if (flag == 0){
        int IsValid = uart6.read();
        if (IsValid == 0){
            return false;
        }
        int x = uart6.read();
        int y = uart6.read();
        SaveVictimXY[0] = x;
        SaveVictimXY[1] = y;
        return true;
    }
    if (flag == 1){
        int IsValid = uart6.read(); 
        if (IsValid == 0){
            return false;
        }
        int x = uart6.read();
        int y = uart6.read();
        DeadVictimXY[0] = x;
        DeadVictimXY[1] = y;
        return true;
    }
    if (flag == 2){
        int x = uart6.read();
        int y = uart6.read();
        int area = uart6.read();
        SaveVictimZone[0] = x;
        SaveVictimZone[1] = y;
        SaveVictimZone[2] = area;
    }
    if (flag == 3){
        int x = uart6.read();
        int y = uart6.read();
        int area = uart6.read();
        DeadVictimZone[0] = x;
        DeadVictimZone[1] = y;
        DeadVictimZone[2] = area;
    }
    return true;
}

// ランダムな位置に移動する
void GoRandomPosition()
{
    tof.getTofValues();
    while (tof.tof_values[2] < 500){
        int angle = random(90,270);
        sts3032.turn(50, angle);
        tof.getTofValues();
    }
    while (tof.tof_values[2] > 500){
        sts3032.drive(50, 0);
        tof.getTofValues();
    }
    
}

void GoNextDetection(){
    if (InEntrance){
        if (NowAngle < 90){
            if (EntrancePositon == 0){
                sts3032.turn(50, -45);
            }
            else if (EntrancePositon == 1){
                sts3032.turn(50, 45);
            }
            else if (EntrancePositon == 2){
                tof.getTofValues();
                if (tof.tof_values[5] < 400){ // 壁方向に回転することを防ぐ　
                    sts3032.turn(50,-135);
                    EntrancePositon = 0;
                }
                else{
                    sts3032.turn(50,-45);
                    EntrancePositon = 1;
                }
            }
            NowAngle += 45;
            return;
        }
        else{
            InEntrance = false;
            GoRandomPosition();
        }
    }
    else if (NowAngle < 360){
        sts3032.turn(50, 45);
        NowAngle += 45;
    }
    else{
        GoRandomPosition();
        NowAngle = 0;

    }
}

void RescueSetup()
{
    VictimDetected = false;
    ZoneDetected = false;
    NearbyVictim = false;
    HaveVictim = false;
    InEntrance = true;
    NowAngle = 0;
    uart6.begin(115200);
}

void RescueLoop()
{
    GetVictimData(0);
    uart1.print(SaveVictimXY[0]);
    uart1.print(' ');
    uart1.println(SaveVictimXY[1]);
    uart1.flush();
    /*
    if (SaveVictimCount < 2){
        if (HaveVictim && !ZoneDetected){
            int MaxI = -1;
            int MaxX = 0;
            int MaxArea = 0;
            for (int i = 0; i < 8; i++){
                GetVictimData(2);
                if (get<2>(SaveVictimZone) > MaxArea){
                    MaxI = i;
                    MaxX = get<0>(SaveVictimZone);
                    MaxArea = get<2>(SaveVictimZone);
                }
            }
            if (MaxI != -1){
                TargetX = get<0>(SaveVictimZone);
                sts3032.turn(50, MaxI * 45 + (MaxX-120) * HFOV);
                DetectedZone = true;
            }
            else{
                GoNextDetection();
            }
            return;
        }
        else if (HaveVictim && ZoneDetected){
            sts3032.drive(50, 0);
            tof.getTofValues(); 
            if (tof.tof_values[1] < 100){
                sts3032.stop();
                sts3032.turn(50, 180);
                servo.BasketOpen();
                delay(1000);
                servo.BasketClose();
                HaveVictim = false;
                ZoneDetected = false;
                SaveVictimCount++;
            }
            return;
        }
        if (!VictimDetected){
            int cnt = 0;
            vector<int> TempSaveVictimXList;
            for (int i = 0; i < 10; i++){
                GetVictimData(0);
                if (SaveVictimList.size() != 0){
                    TempSaveVictimXList.push_back(SaveVictimList[SaveVictimList.size()/2].first);
                }
                cnt += SaveVictimList.size();
            }
            if (cnt < VictimThreshold){ 
                VictimDetected = false;
                GoNextDetection();
            }
            else{ 
                VictimDetected = true;
                TargetX = TempSaveVictimXList[TempSaveVictimXList.size()/2];
                sts3032.turn(50, (TargetX-12) * HFOV);
            }
        }
        else{
            if (!NearbyVictim){
                sts3032.drive(50, 0); // これで精度出なければ P 制御か何かする
                tof.getTofValues(); 
                if (tof.tof_values[2] < 100){
                    sts3032.stop();
                    NearbyVictim = true;
                }
            }
            else{
                servo.HandOpen();
                servo.ArmDown();
                servo.HandClose();
                servo.ArmUp();
                SaveVictimCount++;
                HaveVictim = true;
                NearbyVictim = false;
                VictimDetected = false;
            }
        }
    } 
    else if (DeadVictimCount < 1){
        if (HaveVictim && !ZoneDetected){
            int MaxI = -1;
            int MaxX = 0;
            int MaxArea = 0;
            for (int i = 0; i < 8; i++){
                GetVictimData(3);
                if (get<2>(SaveVictimZone) > MaxArea){
                    MaxI = i;
                    MaxX = get<0>(SaveVictimZone);
                    MaxArea = get<2>(SaveVictimZone);
                }
            }
            if (MaxI != -1){
                TargetX = get<0>(SaveVictimZone);
                sts3032.turn(50, MaxI * 45 + (MaxX-12) * HFOV);
                DetectedZone = true;
            }
            else{
                GoNextDetection();
            }
            return;
        }
        else if (HaveVictim && ZoneDetected){
            sts3032.drive(50, 0);
            tof.getTofValues(); 
            if (tof.tof_values[2] < 100){
                sts3032.stop();
                sts3032.turn(50, 180);
                servo.BasketOpen();
                delay(1000);
                servo.BasketClose();
                HaveVictim = false;
                ZoneDetected = false;
                DeadVictimCount++;
            }
            return;
        }
        if (!VictimDetected){
            int cnt = 0;
            vector<int> DeadSaveVictimXList;
            for (int i = 0; i < 10; i++){
                GetVictimData(1);
                if (DeadVictimList.size() != 0){
                    TempDeadVictimXList.push_back(DeadVictimList[DeadVictimList.size()/2].first);
                }
                cnt += DeadVictimList.size();
            }
            if (cnt < VictimThreshold){ 
                VictimDetected = false;
                GoNextDetection();
            }
            else{ 
                VictimDetected = true;
                TargetX = TempDeadVictimXList[TempSaveVictimXList.size()/2];
                sts3032.turn(50, (TargetX-12) * HFOV);
            }
        }
        else{
            if (!NearbyVictim){
                sts3032.drive(50, 0); 
                tof.getTofValues(); 
                if (tof.tof_values[2] < 100){
                    sts3032.stop();
                    NearbyVictim = true;
                }
            }
            else{
                servo.HandOpen();
                servo.ArmDown();
                servo.HandClose();
                servo.ArmUp();
                DeadVictimCount++;
                HaveVictim = true;
                NearbyVictim = false;
                VictimDetected = false;
            }
        }
    } 
    */
}