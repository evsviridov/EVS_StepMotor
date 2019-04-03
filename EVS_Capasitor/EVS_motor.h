#pragma once

#ifndef __EVS_MOTOR_H
#define __EVS_MOTOR_H

#include <Arduino.h>

#define TRUE (1)
#define FALSE (0)
#define true TRUE
#define false FALSE

struct EVS_motor_config
{
    long _step = 400;
    long _microstep = 1;
    float _max_rps = 10.0;
    int _ID;
};

struct EVS_motor_pin_config
{
    int _ena;
    int _pul;
    int _dir;
};

struct EVS_motor_pin_inversion
{
    bool _ena = FALSE;
    bool _pul = FALSE;
    bool _dir = FALSE;
};

struct EVS_motor_pin_truelevel
{
    bool _ena = TRUE;
    bool _pul = TRUE;
    bool _dir = TRUE;
};

struct EVS_motor_pin_state
{
    bool _ena;
    bool _pul;
    bool _dir;
};

struct EVS_motor_state
{
    bool _ena;
    bool _dir;
    long _counter = 0;
};

struct EVS_motor_timeconfig
{
    float _rps = 1.0;
    float _rpm = _rps * 60.0;
    float _pulse_freq;
    long _pulse_per_rev;
    unsigned long _pulse_period_us;
};

struct EVS_motor_time_state
{
    unsigned long _last_pulse_us;
    unsigned long _next_pulse_us;
};

class EVS_motor
{
private:
    EVS_motor_config _motor_config;
    EVS_motor_pin_config _pin_config;
    EVS_motor_pin_inversion _pin_inversion;
    EVS_motor_timeconfig _time_config;

    EVS_motor_state _state;
    EVS_motor_time_state _time_state;
    EVS_motor_pin_state _pin_state;

    EVS_motor_pin_truelevel _pin_truelevel;


    void __doPUL(void);
    void __doENA(bool level);
    void __doDIR(bool level);
    void _setupPin(void);

public:
    EVS_motor();
    EVS_motor(EVS_motor_config motor_config, EVS_motor_pin_config pins, EVS_motor_pin_inversion inversion);

    void config(EVS_motor_config motor_config, EVS_motor_pin_config pins, EVS_motor_pin_inversion inversion);
    void config_motor(EVS_motor_config config);
    void config_pins(EVS_motor_pin_config pins);
    void config_invertpin(EVS_motor_pin_inversion inversion);
    void setID(int id);
    int getID(void);

    int init(void);
    
    bool getEna(void);
    void setEna(bool value);
    bool getDir(void);
    void setDir(bool value);

    int pul(void);
    unsigned long setLastPulTimeUs(void);

    float getRps(void);
    void setRps(float rps);
    float getRpsMax(void);
    void setRpsMax(float rps);

    long getCounter(void);
    void setCounter(long counter);
    long getPulsesPerRev(void);
    int isTimeElapsed(void);
 
};

#endif __EVS_MOTOR_H