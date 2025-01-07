#include "Microservo.h"

Microservo::Microservo(HardwareSerial *serial)
{
    _serial = serial;
}

void Microservo::HandClose()
{
    _serial->println("HandClose");
}

void Microservo::HandOpen()
{
    _serial->println("HandOpen");
}

void Microservo::ArmUp()
{
    _serial->println("ArmUp");
}

void Microservo::ArmDown()
{
    _serial->println("ArmDown");
}

void Microservo::BasketClose()
{
    _serial->println("BasketClose");
}

void Microservo::BasketOpen()
{
    _serial->println("BasketOpen");
}

void Microservo::AttachServo()
{
    _serial->println("AttachServo");
}

void Microservo::DetachServo()
{
    _serial->println("DetachServo");
}

void Microservo::initPos()
{
    AttachServo();
    BasketClose();
    HandClose();
    ArmUp();
    delay(500);
    DetachServo();
}

