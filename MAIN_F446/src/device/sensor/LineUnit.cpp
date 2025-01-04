#include "LineUnit.h"

LineUnit::LineUnit(HardwareSerial *serial)
{
    _serial = serial;
    _serial->begin(115200);
    init();
}

void LineUnit::init()
{
    for (int i = 0; i < 4; i++)
    {
        colorLTime[i] = 0;
        colorRTime[i] = 0;
    }
    LastColorL = 4;
    LastColorR = 4;
    Flash();
}

void LineUnit::Flash()
{
    while (_serial->available())
    {
        _serial->readStringUntil('\n');
    }
}
int LineUnit::read()
{
    int returnCode = 0;
    if (_serial->available())
    {
        String str = _serial->readStringUntil('\n');
        str = _serial->readStringUntil('\n');

        int dataBox[30];
        StringToIntValues(str, dataBox);
        for (int i = 0; i < 15; i++)
        {
            _photoReflector[i] = dataBox[i];
        }
        _frontPhotoReflector = dataBox[15];
        for (int i = 0; i < 3; i++)
        {
            colorL[i] = dataBox[16 + i];
        }
        for (int i = 0; i < 3; i++)
        {
            colorR[i] = dataBox[19 + i];
        }

        Flash();
        checkColor(colorL, colorLTime, &LastColorL);
        checkColor(colorR, colorRTime, &LastColorR);

        returnCode = 1;
    }
    return returnCode;
}

void LineUnit::setBrightness(int brightness)
{
    _serial->write(brightness);
}

void LineUnit::StringToIntValues(String str, int values[])
{
    int i = 0;
    int j = 0;
    while (i < str.length())
    {
        if (j > 29)
        {
            break;
        }
        if (str[i] == ' ')
        {
            i++;
            continue;
        }
        String value = "";
        while (str[i] != ' ' && i < str.length())
        {
            value += str[i];
            i++;
        }
        values[j] = value.toInt();
        j++;
    }
}

void LineUnit::checkColor(int colorArr[], unsigned long colorTime[], int *LastColor)
{
    int color = 0;                              // Black
    if (colorArr[0] > 200 && colorArr[1] > 200) // White
    {
        color = 1;
    }
    else if ((float)colorArr[0] *1.3f < colorArr[1]) // Green
    {

        color = 2;
    }
    else if (colorArr[0] > colorArr[1] + 25) // Red
    {
        color = 3;
    }
    else if(colorArr[0] < 80 && colorArr[1] < 80){
        color = 0;
    }
    else{
        color = 1;
    }

    if (color != *LastColor % 5)
    {
        *LastColor = color;
    }
    else
    {
        if (*LastColor / 5 > -1)
        {
            colorTime[color] = millis();
        }
        else
        {
            *LastColor += 5;
        }
    }
}