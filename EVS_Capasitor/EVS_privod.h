#pragma once

#ifndef __EVS_PRIVOD_H
#define __EVS_PRIVOD_H

#include "EVS_motor.h"

#define ERR_POSITION_MIN (0.03)
#define ERR_POSITION_MAX (0.08)

#define IS_LESS(a, b, eps) ((a) < ((b) - (eps)))
#define IS_MORE(a, b, eps) ((a) > ((b) + (eps)))
#define IS_EQUAL(a, b, eps) (fabs((a) - (b)) < (eps))

struct EVS_privod_status
{
    int _ID;
    bool _isIdle;
    bool _isMoving;
    bool _isAutoMove;
    bool _isOnTarget;
    bool _isInsideRange;
    bool _isLoSide;
    bool _isHiSide;
};

typedef int EVS_privod_function(class EVS_privod *privod);
struct EVS_privod_functions
{
    EVS_privod_function *_beforeInitFunc = NULL;
    EVS_privod_function *_beforePULFunc = NULL;
    EVS_privod_function *_afterPULFunc = NULL;
    EVS_privod_function *_beforeENAFunc = NULL;
    EVS_privod_function *_beforeDIRFunc = NULL;
    EVS_privod_function *_measureCurrentPositionFunc = NULL;
    EVS_privod_function *_isOnTargetFunc = NULL;
};

struct EVS_privod_position
{
    long _current_long;
    float _current_float;
    long _target_long;
    float _target_float;
};

struct EVS_privod_counterorigin
{
    long _origin = 0;
    float _k = 1;
    float _b = 0;
};

struct EVS_privod_position_error
{
    float _diff, _diff_prev;
    long _time, _time_prev;
    float _min_error = ERR_POSITION_MIN;
    float _max_error = ERR_POSITION_MAX;
};

struct EVS_privod_rangepoint
{
    float _analog;
    float _rotate;
};

struct EVS_privod_range
{
    EVS_privod_rangepoint _p_min;
    EVS_privod_rangepoint _p_max;
    EVS_privod_range(void)
    {
        _p_min={0,0};
        _p_max={100,1};
    }
    EVS_privod_range(EVS_privod_rangepoint p_min, EVS_privod_rangepoint p_max)
    {
        _p_min = p_min;
        _p_max = p_max;
    };
};

typedef enum
{
    MANUAL,
    AUTO
} EVS_privod_movemode;

float rangeMap(float x, float in_min, float in_max, float out_min, float out_max);

class EVS_privod : public EVS_motor
{
  private:
    EVS_privod_functions _privod_functions;
    EVS_privod_status _privod_status;
    EVS_privod_position _privod_position;
    EVS_privod_position_error _position_error;
    EVS_privod_counterorigin _privod_counterorigin;
    EVS_privod_movemode _movemode = AUTO;
    EVS_privod_range _privod_range;

  public:
    void setBeforeInitFunc(EVS_privod_function *func);
    void setBeforePulFunc(EVS_privod_function *func);
    void setAfterPulFunc(EVS_privod_function *func);
    void setBeforeEnaFunc(EVS_privod_function *func);
    void setMeasurePositionFunc(EVS_privod_function *func);
    void setIsOnTargetFunc(EVS_privod_function *func);

    int init(void);

    void setEna(bool value);
    void setDir(bool value);
    int pul(void);

    int isOnTarget(void);

    void setMoveMode(EVS_privod_movemode movemode);
    EVS_privod_movemode getMoveMode(void);

    long getCurrentPositionLong(void);
    void setCurrentPositionLong(long position);
    float getCurrentPositionFloat(void);
    void setCurrentPositionFloat(float position);

    void setTargetPositionFloat(float targetPosition);
    float getTargetPositionFloat(void);
    void setDirectionToTargetFloat(void);
    float getPositionErrorFloat(void);
    void setPositionErrorRangeFloat(float min_error = ERR_POSITION_MIN, float max_error = ERR_POSITION_MAX);

    void setTargetPositionLong(long targetPosition);
    long getTargetPositionLong(void);
    void setDirectionToTargetLong(void);
    long getPositionErrorLong(void);
    void setPositionErrorRangeLong(long min_error = 0, long max_error = 1);

    void setCounterOrigin(long origin = 0);
    long getCounterOrigin(void);
    void setOriginFromPosition(long position);
    void setOriginFromRotate(float rotate);
    long getPositionFromOrigin(void);

    float computePositionToRotate(long position);
    long computeRotateToPosition(float rotate);

    void setRangeAnalogRotate(EVS_privod_range range);
    float mapAnalogToRotate(float analog);
    float mapRotateToAnalog(float rotate);
};

#endif __EVS_PRIVOD_H
