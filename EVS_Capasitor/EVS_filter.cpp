#include "EVS_filter.h"

float filterMedian(float *v, int num = 3)
{
    float middle;
    float a, b, c;
    a = v[0];
    b = v[1];
    c = v[2];
    if ((a <= b) && (a <= c))
    {
        middle = (b <= c) ? b : c;
    }
    else
    {
        if ((b <= a) && (b <= c))
        {
            middle = (a <= c) ? a : c;
        }
        else
        {
            middle = (a <= b) ? a : b;
        }
    }
    return middle;
}

float filterMean(float *v, int num)
{
    int sum = 0;
    for (int i = 0; i < num; ++i)
        sum += v[i];
    return sum / num;
}

float filterRunning(float *v, int num)
{
    float filtered=0;
    const float k1=0.1;
    const float k2=1.0-k1;
    filtered=v[0];
    for (int i = 1; i < num; ++i)
        filtered = k1*v[i]+k2*filtered;
    return filtered;
}
