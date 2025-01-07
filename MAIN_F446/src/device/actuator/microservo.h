#ifndef microservo_H
#define microservo_H

#include <Arduino.h>

class Microservo
{
public:
    Microservo(HardwareSerial *serial);
    void HandClose();
    void HandOpen();
    void ArmUp();
    void ArmDown();
    void BasketClose();
    void BasketOpen();
    void AttachServo();
    void DetachServo();

    void initPos();

private:
    HardwareSerial *_serial;

};

#endif