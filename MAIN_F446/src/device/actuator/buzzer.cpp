#include "buzzer.h"

#define C3 131
#define D3 147
#define E3 165
#define F3 175
#define G3 196
#define A3 220
#define B3 247
#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define B4 494
#define C5 523
#define D5 587
#define E5 659
#define F5 698
#define G5 784
#define A5 880
#define B5 988
#define C6 1047

Buzzer::Buzzer(int pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
}

void Buzzer::setFrequency(int freq)
{
    if (isDisabled)
    {
        return;
    }

    analogWriteFrequency(freq);
    analogWrite(_pin, 64);
}

void Buzzer::mute()
{
    analogWrite(_pin, 0);
}

void Buzzer::beep(int note, double duration)
{
    int interbal = 10;
    long beepDuration = 60000.0 / _bpm * duration - interbal;
    setFrequency(note);
    delay(beepDuration);
    mute();
    delay(interbal);
}

void Buzzer::boot()
{
    _bpm = 400;

    beep(C5, 0.5);
    beep(E5, 0.5);
    beep(G5, 0.5);
}

void Buzzer::ObjectDetected()
{
    _bpm = 200;
    for (int i = 0; i < 2; i++)
    {
        beep(E4, 0.5);
        beep(A4, 0.5);
        beep(G4, 0.5);
    }
}

void Buzzer::GreenMarker(int p)
{
    _bpm = 100;
    beep(C5, 0.5);
    if (p > 0)
    {
        beep(G5, 0.5);
    }
    if (p > 1)
    {
        beep(E5, 0.5);
    }
    if (p > 2)
    {
        beep(C5, 0.5);
    }
}

void Buzzer::kouka()
{
    _bpm = 100;
    // とよあしはらの
    beep(C4, 1);
    beep(E4, 1);
    beep(G4, 1.5);
    beep(F4, 0.5);

    beep(E4, 1);
    beep(C4, 1);
    beep(D4, 2);

    // ちゅうげんと
    beep(E4, 1);
    beep(E4, 1);
    beep(D4, 1.5);
    beep(C4, 0.5);

    beep(G4, 4);

    // ひらきましけん
    beep(E4, 1);
    beep(E4, 1);
    beep(D4, 1.5);
    beep(C4, 0.5);

    beep(A3, 1);
    beep(C4, 1);
    beep(G3, 2);

    // やまとたけ
    beep(C4, 1);
    beep(E4, 1);
    beep(D4, 1.5);
    beep(G3, 0.5);

    beep(C4, 4);
    delay(10000);
}