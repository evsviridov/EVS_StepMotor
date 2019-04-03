#include "EVS_privod.h"

int EVS_privod::init(void)
{
    int err;
    if (_privod_functions._beforeInitFunc != NULL)
    {
        err = _privod_functions._beforeInitFunc(this);
        if (err)
            return err;
    }
    EVS_motor::init();
}

void EVS_privod::setEna(bool value)
{
    if (_privod_functions._beforeENAFunc != NULL)
    {
        int err = _privod_functions._beforeENAFunc(this);
        if (err)
            return err;
    }
    EVS_motor::setEna(value);
}

void EVS_privod::setDir(bool value)
{
    if (_privod_functions._beforeDIRFunc != NULL)
    {
        int err = _privod_functions._beforeDIRFunc(this);
        if (err)
            return err;
    }
    EVS_motor::setDir(value);
}

int EVS_privod::pul(void)
{
    int status;
    if (_privod_functions._beforePULFunc != NULL)
    {
        status = _privod_functions._beforePULFunc(this);
        if (status)
            return status;
    }
    EVS_motor::pul();
    if (_privod_functions._afterPULFunc != NULL)
    {
        status = _privod_functions._afterPULFunc(this);
        if (status)
            return status;
    }
    return 0;
}
float EVS_privod::computePositionToRotate(long position)
{
    return (float)position / (float)getPulsesPerRev();
}
long EVS_privod::computeRotateToPosition(float rotate)
{
    return rotate * getPulsesPerRev();
}
int EVS_privod::isOnTarget(void)
{
    if (_privod_functions._isOnTargetFunc != NULL)
    {
        _privod_status._isOnTarget = _privod_functions._isOnTargetFunc(this);
    }
    else
    {
        _privod_status._isOnTarget = (getPositionErrorLong() == 0);
    }
    return _privod_status._isOnTarget;
}

void EVS_privod::setMoveMode(EVS_privod_movemode movemode)
{
    _movemode = movemode;
    if (movemode == MANUAL)
    {
        setEna(false);
    }
}

EVS_privod_movemode EVS_privod::getMoveMode(void)
{
    return _movemode;
}

void EVS_privod::setCurrentPositionLong(long position)
{
    setOriginFromPosition(position);
}
void EVS_privod::setTargetPositionLong(long targetPosition)
{
    _privod_position._target_long = targetPosition;
    _privod_position._target_float = computePositionToRotate(targetPosition);
    _privod_status._isOnTarget = FALSE;
    setDirectionToTargetLong();
}
long EVS_privod::getCurrentPositionLong(void)
{
    if (_privod_functions._measureCurrentPositionFunc != NULL)
    {
        int err = _privod_functions._measureCurrentPositionFunc(this);
        if (err)
            return err;
    }
    return getPositionFromOrigin();
}
long EVS_privod::getTargetPositionLong(void)
{
    return _privod_position._target_long;
}
long EVS_privod::getPositionErrorLong(void)
{
    return getCurrentPositionLong() - getTargetPositionLong();
}

float EVS_privod::getCurrentPositionFloat(void)
{
    return computePositionToRotate(getCurrentPositionLong());
}
void EVS_privod::setCurrentPositionFloat(float rotate)
{
    setCurrentPositionLong(computeRotateToPosition(rotate));
}
void EVS_privod::setTargetPositionFloat(float targetRotate)
{
    setTargetPositionLong(computeRotateToPosition(targetRotate));
    _position_error._diff_prev = getPositionErrorFloat();
}
float EVS_privod::getTargetPositionFloat(void)
{
    return computePositionToRotate(getTargetPositionLong());
}

float EVS_privod::getPositionErrorFloat(void)
{
    _position_error._diff = getCurrentPositionFloat() - getTargetPositionFloat();
    return _position_error._diff;
}

void EVS_privod::setPositionErrorRangeFloat(float min_error, float max_error)
{
    _position_error._min_error = min_error;
    _position_error._max_error = max_error;
}

void EVS_privod::setDirectionToTargetLong(void)
{
    long err = getPositionErrorLong();
    if (err < 0)
    {
        if (!getDir())
            setDir(TRUE);
    }
    else if (err > 0)
    {
        if (getDir())
            setDir(FALSE);
    }
}

void EVS_privod::setDirectionToTargetFloat(void)
{
    float err = getPositionErrorFloat();
    if (err < _position_error._min_error)
    {
        if (!getDir())
            setDir(TRUE);
    }
    else if (err > (-_position_error._min_error))
    {
        if (getDir())
            setDir(FALSE);
    }
}

void EVS_privod::setBeforeInitFunc(EVS_privod_function *func)
{
    _privod_functions._beforeInitFunc = func;
}

void EVS_privod::setBeforePulFunc(EVS_privod_function *func)
{
    _privod_functions._beforePULFunc = func;
}

void EVS_privod::setAfterPulFunc(EVS_privod_function *func)
{
    _privod_functions._afterPULFunc = func;
}

void EVS_privod::setBeforeEnaFunc(EVS_privod_function *func)
{
    _privod_functions._beforeENAFunc = func;
}

void EVS_privod::setMeasurePositionFunc(EVS_privod_function *func)
{
    _privod_functions._measureCurrentPositionFunc = func;
}
void EVS_privod::setIsOnTargetFunc(EVS_privod_function *func)
{
    _privod_functions._isOnTargetFunc = func;
}

void EVS_privod::setCounterOrigin(long origin = 0)
{
    _privod_counterorigin._origin = origin;
}
long EVS_privod::getCounterOrigin(void)
{
    return _privod_counterorigin._origin;
}

void EVS_privod::setOriginFromPosition(long position)
{
    setCounterOrigin(getCounter() - position);
}
void EVS_privod::setOriginFromRotate(float rotate)
{
    setOriginFromPosition(computeRotateToPosition(rotate));
}

long EVS_privod::getPositionFromOrigin(void)
{
    return getCounter() - getCounterOrigin();
}

void EVS_privod::setRangeAnalogRotate(EVS_privod_range range)
{
    _privod_range = range;
}

float rangeMap(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float EVS_privod::mapAnalogToRotate(float analog)
{
    //  return rangeMap(percent, 90, 10, 0, 40); // Capasitors
    return rangeMap(analog, _privod_range._p_min._analog, _privod_range._p_max._analog,
                    _privod_range._p_min._rotate, _privod_range._p_max._rotate);
    //9.2, 49.2, 0, 40); // Zond
}

float EVS_privod::mapRotateToAnalog(float rotate)
{
    //  return rangeMap(rotate, 0, 40, 90, 10); // Capasitors
    return rangeMap(rotate, _privod_range._p_min._rotate, _privod_range._p_max._rotate,
                    _privod_range._p_min._analog, _privod_range._p_max._analog);
    //  return rangeMap(rotate, 0, 40, 9.2, 49.2); // Zond
}
