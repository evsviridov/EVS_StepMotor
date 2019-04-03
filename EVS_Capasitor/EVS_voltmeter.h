#pragma once

#ifndef __EVS_VOLTMETER_H
#define __EVS_VOLTMETER_H

#include <Arduino.h>

#define ADC_RESOLUTION 10
const int ADC_RANGE = ((1 << ADC_RESOLUTION) - 1);

int doVoltmeter(int pin, float *fValue = NULL, int mSec = 20);
float potentiometer(int pin);

#endif __EVS_VOLTMETER_H