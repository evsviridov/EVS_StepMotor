#pragma once

#ifndef __EVS_FILTER_H
#define __EVS_FILTER_H

float filterMedian(float *v, int num = 3);
float filterMean(float *v, int num);
float filterRunning(float *v, int num);


#endif __EVS_FILTER_H
