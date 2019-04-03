#include "Arduino.h"
#include "EVS_motor.h"

void EVS_motor::__doPUL(void)
{
    bool level;
    level = _pin_truelevel._pul;
    digitalWrite(_pin_config._pul, level);
    delayMicroseconds(5);
    digitalWrite(_pin_config._pul, !level);
}

void EVS_motor::__doENA(bool level)
{
    digitalWrite(_pin_config._ena, level);
    delayMicroseconds(100);
}

void EVS_motor::__doDIR(bool level)
{
    digitalWrite(_pin_config._dir, level);
    delayMicroseconds(100);
}

void EVS_motor::_setupPin(void)
{
    pinMode(_pin_config._ena, OUTPUT);
    pinMode(_pin_config._pul, OUTPUT);
    pinMode(_pin_config._dir, OUTPUT);
}

EVS_motor::EVS_motor()
{
}

EVS_motor::EVS_motor(EVS_motor_config motor_config, EVS_motor_pin_config pins, EVS_motor_pin_inversion inversion)
{
    config(motor_config, pins, inversion);
    init();
}

void EVS_motor::config(EVS_motor_config motor_config, EVS_motor_pin_config pins, EVS_motor_pin_inversion inversion)
{
    config_motor(motor_config);
    config_pins(pins);
    config_invertpin(inversion);
}

void EVS_motor::config_motor(EVS_motor_config config)
{
    _motor_config = config;
//    _motor_config._step = config._step;
//    _motor_config._microstep = config._microstep;
//    _motor_config._max_rps = config._max_rps;
    _time_config._pulse_per_rev = _motor_config._step * _motor_config._microstep;
}

void EVS_motor::config_pins(EVS_motor_pin_config pins)
{
    _pin_config = pins;
//    _pin_config._ena = pins._ena;
//    _pin_config._pul = pins._pul;
//    _pin_config._dir = pins._dir;
}

void EVS_motor::config_invertpin(EVS_motor_pin_inversion inversion)
{
    _pin_inversion = inversion;
//    _pin_truelevel._ena = !_pin_inversion._ena;
//    _pin_truelevel._pul = !_pin_inversion._pul;
//    _pin_truelevel._dir = !_pin_inversion._dir;
}

void EVS_motor::setID(int id)
{
    _motor_config._ID=id;
}
int EVS_motor::getID(void)
{
    return _motor_config._ID;
}

int EVS_motor::init(void)
{
    _setupPin();
}

bool EVS_motor::getEna(void)
{
    return _state._ena;
}

void EVS_motor::setEna(bool value)
{
    _state._ena = value;
    _pin_state._ena = _pin_inversion._ena ? !_state._ena : _state._ena;
    __doENA(_pin_state._ena);
}

bool EVS_motor::getDir(void)
{
    return _state._dir;
}

void EVS_motor::setDir(bool value)
{
    _state._dir = value;
    _pin_state._dir = _pin_inversion._dir ? (!_state._dir) : _state._dir;
    __doDIR(_pin_state._dir);
}

int EVS_motor::pul(void)
{
    __doPUL();
    if (_state._dir)
        ++_state._counter;
    else
        --_state._counter;
    _time_state._last_pulse_us = micros();
    _time_state._next_pulse_us = _time_state._last_pulse_us + _time_config._pulse_period_us;
    return 0;
}
unsigned long EVS_motor::setLastPulTimeUs(void)
{
    _time_state._last_pulse_us = micros();
    return _time_state._last_pulse_us;
}

int EVS_motor::isTimeElapsed(void)
{
	#define MAX_U32	((unsigned long) 0xFFFFFFFF)
	unsigned long current_us, delta_us;
	current_us=micros();
	if(current_us >=_time_state._last_pulse_us)
	{
		delta_us=(current_us-_time_state._last_pulse_us);
	}
	else
	{
		delta_us=current_us+(MAX_U32-_time_state._last_pulse_us);
	}
	return (delta_us >= _time_config._pulse_period_us);
	#undef MAX_U32
//    return micros() >= _time_state._next_pulse_us;
}

float EVS_motor::getRps(void)
{
    return _time_config._rps;
}
void EVS_motor::setRps(float rps)
{
    rps = fmin(fabs(rps), _motor_config._max_rps);
    _time_config._rps = rps;
    _time_config._rpm = 60.0 * _time_config._rps;
    _time_config._pulse_freq = _time_config._rps * _time_config._pulse_per_rev;
    _time_config._pulse_period_us = 1.0E6 * 1.0 / _time_config._pulse_freq;
}

float EVS_motor::getRpsMax(void)
{
    return _motor_config._max_rps;
}
void EVS_motor::setRpsMax(float rps)
{
    _motor_config._max_rps = rps;
    setRps(getRps());
}

long EVS_motor::getCounter(void)
{
    return _state._counter;
}
void EVS_motor::setCounter(long counter)
{
    _state._counter = counter;
}
long EVS_motor::getPulsesPerRev(void)
{
    return _time_config._pulse_per_rev;
}
