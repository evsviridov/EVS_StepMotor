#include "EVS_voltmeter.h"
#include "EVS_filter.h"

int doVoltmeter(int pin, float *fValue = NULL, int mSec = 20)
{
    unsigned long starttime, endtime, delayUs;

    long lValue = 0;
    long n = 0;
    float result;
    int v;

    delayUs = (unsigned long)mSec * 1000UL;
    starttime = micros();
    while((micros() - starttime) <= delayUs)
    {
        v=analogRead(pin);
        lValue += v;
        ++n;
        randomSeed(random(v));
        delayMicroseconds(random(5));
    }
    if (fValue)
    {
        *fValue = ((float)lValue) / ((float)n) / ADC_RANGE;
    }
    return (int)(lValue / n);
}

float potentiometer(int pin)
{
  float result;
  const int num = 5;
  float v[num];
  for (int i = 0; i < num; ++i)
  {
    v[i] = analogRead(pin);
  }
  // result = filterMedian(v, 3);
  result = filterMean(v, num);
  // result = filterRunning(v, num);
  return (float)result / 1023.0 * 100.0;
}


