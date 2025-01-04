#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer
{
public:
    Buzzer(int pin);
    void beep(int note, double duration);
    void mute();
    void kouka();
    void boot();
    void ObjectDetected();
    void GreenMarker(int p);
    const bool isDisabled = false;

private:
    int _pin;
    int _bpm;
    void setFrequency(int freq);
};

#endif