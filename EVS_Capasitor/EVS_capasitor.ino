#define _VERSION_ "EVS_Capasitor\tVER=0.1.0\tDATE=17-Dec-2018"

#include <Arduino.h>
#include "EVS_CmdParser.h"
#include "EVS_voltmeter.h"
#include "EVS_motor.h"
#include "EVS_privod.h"

EVS_CmdParser parser;
#define MOT_CMD ("MOT")
const char *KAVYCHKA = "\"";
bool _isIdleResponse = FALSE;

#define MOT_MAXRPS 3
#define MOT_STEPPERREV 400
#define MOT_MICROSTEP 1
#define MOT_INVENA 1
#define MOT_INVPUL 0
#define MOT_INVDIR 1

#define NUM_PRIVOD (6)

EVS_motor_pin_config pin0 = {14, 15, 16};
EVS_motor_pin_config pin1 = {17, 18, 19};
EVS_motor_pin_config pin2 = {20, 21, 22};
EVS_motor_pin_config pin3 = {23, 24, 25};
EVS_motor_pin_config pin4 = {26, 27, 28};
EVS_motor_pin_config pin5 = {29, 30, 31};

#define ZOND_RANGE EVS_privod_range({9.2, 0}, {49.2, 40})
#define SHIBER_RANGE EVS_privod_range({21.67, 0}, {27.86, 11})
#define CAPASITOR_RANGE EVS_privod_range({90, 0}, {10, 40})


EVS_motor_pin_config pin_config[NUM_PRIVOD] = {pin0, pin1, pin2, pin3, pin4, pin5}; // ENA, PUL, DIR
EVS_motor_config motor_config[NUM_PRIVOD];
EVS_motor_pin_inversion pin_inversion[NUM_PRIVOD];
float max_rps[NUM_PRIVOD] = {3, 2, 1, 1, 1, 1};
EVS_privod_range privod_range[NUM_PRIVOD] = {
    ZOND_RANGE,
    SHIBER_RANGE,
    CAPASITOR_RANGE,
    CAPASITOR_RANGE,
    CAPASITOR_RANGE,
    CAPASITOR_RANGE};
int EVS_motor_analog_pin[NUM_PRIVOD] = {A0, A1, A2, A3, A4, A5}; //Potentiometer pin
float value_percent[NUM_PRIVOD];
class EVS_privod privod[NUM_PRIVOD];

class HardwareSerial *stream = &Serial;

void streamDoPeriodically(String &response);

float doMeasPercent(int num, int aperture = 200)
{
  float v;
  doVoltmeter(EVS_motor_analog_pin[num], &v, aperture);
  return v;
}

void doMeasPercentAll(void)
{
  int i;
  float v;
  for (i = 0; i < NUM_PRIVOD; ++i)
  {
    doVoltmeter(EVS_motor_analog_pin[i], &v, 400);
    value_percent[i] = v * 100;
  }
}

void doMeasAndRenewPositionAll(void)
{
  int i;
  float rotate;
  long position;
  String s;
  doMeasPercentAll();
  for (i = 0; i < NUM_PRIVOD; ++i)
  {

    //  rotate = percent2rotate(value_percent[i]);
    rotate = privod[i].mapAnalogToRotate(value_percent[i]);
    position = privod[i].computeRotateToPosition(rotate);
    privod[i].setCurrentPositionLong(position);
    s = "RENEW:" + String(i);
    s += "\tPERCENT=" + String(value_percent[i], 2);
    s += "\tf_POS=" + String(rotate, 2);
    s += "\tl_POS=" + String(position);
    parser.sendResponse(0, s);
  }
}

void doSetupPositionAll(void)
{
  doMeasAndRenewPositionAll();
  for (int i = 0; i < NUM_PRIVOD; ++i)
  {
    privod[i].setTargetPositionLong(privod[i].getCurrentPositionLong());
  }
}

int measureCurrentPosition_function(class EVS_privod *p)
{
  long position = p->getCurrentPositionLong();
  float rotate = p->computePositionToRotate(position);
  float percent = p->mapRotateToAnalog(rotate); //   rotate2percent(rotate);
  // p->setCurrentPositionLong(position);
  return 0;
}

int measureCurrentPercent_function(class EVS_privod *p)
{
  static long measureTime = 0;
  static int isFirst = 1;
  if (isFirst || millis() >= measureTime)
  {
    isFirst = 0;
    measureTime = millis() + 10;
    for (int i = 0; i < NUM_PRIVOD; ++i)
    {
      if (p == &privod[i])
      {
        float value = potentiometer(EVS_motor_analog_pin[i]);
        p->setCurrentPositionFloat(p->mapAnalogToRotate(value)); //  percent2rotate(value));
      }
    }
  }
  return 0;
}

int gotoRotate_function(class EVS_privod *p)
{
  int isNoPul = 1;

  if (p->isOnTarget())
  {
    if (p->getEna())
      p->setEna(FALSE);
    isNoPul = 1;
  }
  else
  {
    float error = p->computePositionToRotate(p->getPositionErrorLong());
    if (fabs(error) >= 0.2)
      p->setRps(p->getRps() + 0.02);
    else
      p->setRps(fmax(0.2, p->getRps() - 0.02));
    p->setDirectionToTargetLong();
    isNoPul = 0;
  }
  return isNoPul;
}

int gotoTarget_function(class EVS_privod *p)
{
  int isNoPul = 0;
  if (p->isOnTarget())
  {
    if (p->getEna())
      p->setEna(FALSE);
    isNoPul = 1;
  }
  else
  {
    float error = fabs(p->getPositionErrorFloat());
    if (error > 1)
      p->setRps(p->getRps() + 0.003);
    else
      p->setRps(fmax(0.1, p->getRps() - 0.002));
    p->setDirectionToTargetFloat();
    isNoPul = 0;
  }
  return isNoPul;
}

int tuda_suda_function(class EVS_privod *p)
{
  long count = p->getCounter();
  float position;
  p->setRps(0.5);
  position = p->getCurrentPositionFloat();
  if (position >= 100 && p->getDir() == 1)
  {
    p->setDir(0);
    delay(200);
    return 0;
  }
  if (position <= 0 && p->getDir() == 0)
  {
    p->setDir(1);
    delay(200);
    return 0;
  }
  return 0;
}

void setup_privod()
{
  int i;
  for (i = 0; i < NUM_PRIVOD; ++i)
  {
    motor_config[i]._ID = i;
    motor_config[i]._step = MOT_STEPPERREV;
    motor_config[i]._microstep = MOT_MICROSTEP;
    motor_config[i]._max_rps = max_rps[i];
    pin_inversion[i]._ena = MOT_INVENA;
    pin_inversion[i]._pul = MOT_INVPUL;
    pin_inversion[i]._dir = MOT_INVDIR;

    privod[i].config(motor_config[i], pin_config[i], pin_inversion[i]);

    privod[i].setBeforePulFunc(gotoRotate_function);
    // privod[0].setBeforePulFunc(tuda_suda_function);
    // privod[i].setMeasurePositionFunc(measureCurrentPercent_function);
    // privod[i].setMeasurePositionFunc(measureCurrentPosition_function);

    privod[i].init();
    privod[i].setEna(0);
    privod[i].setDir(0);
    privod[i].setRps(max_rps[i]);
    privod[i].setPositionErrorRangeFloat(0.05, 0.1);
    privod[i].setRangeAnalogRotate(privod_range[i]);
  }
  //  privod[0].setRangeAnalogRotate(zond_range);
  doSetupPositionAll();
}

void streamDoPeriodically(String &s)
{
  int i;
  bool current = 1, target = 1, count = 0, idle = 1;
  s = "STATUS: ";
  if (current)
  {
    s += "ROTATE=";
    for (i = 0; i < NUM_PRIVOD - 1; ++i)
    {
      s += String(privod[i].getCurrentPositionFloat(), 1);
      s += ";";
    }
    s += String(privod[i].getCurrentPositionFloat(), 1);
  }

  if (target)
  {
    s += "\tTARGET=";
    for (i = 0; i < NUM_PRIVOD - 1; ++i)
    {
      s += String(privod[i].getTargetPositionFloat(), 1);
      s += ";";
    }
    s += String(privod[i].getTargetPositionFloat(), 1);
  }

  if (count)
  {
    s += "\tCOUNT=";
    for (i = 0; i < NUM_PRIVOD - 1; ++i)
    {
      s += String(privod[i].getCounter());
      s += ";";
    }
    s += String(privod[i].getCounter());
  }
  if (idle)
  {
    s += String("\tIDLE=") + (_isIdleResponse ? "1" : "0");
  }
}

int doABORT(String &param, String &response)
{
  setup_privod();
  response = "ABORT:";
  return 0;
}

int doRESET(String &param, String &response)
{
  setup_privod();
  response = "RESET:";
  return 0;
}
int doRENEW(String &param, String &response)
{
  doSetupPositionAll();
  response = "RENEW:";
  return 0;
}

int doGOTO(String &param, String &response)
{
  String position, privod_str;
  int num_privod = 0, start, count;
  response = "";
  _EVS_CMDPARSERCLASS::splitCmdParam(param, position, privod_str);
  if (privod_str.length() <= 0)
    num_privod = NUM_PRIVOD + 1;
  else
    num_privod = min(NUM_PRIVOD - 1, abs(privod_str.toInt()));
  if (param == "?")
  {
    streamDoPeriodically(response);
  }
  else
  {
    response = String("GOTO: privod=");
    if (num_privod > NUM_PRIVOD)
    {
      start = 0;
      count = NUM_PRIVOD;
      response += "ALL\t";
    }
    else
    {
      start = num_privod;
      count = 1;
      response += String(num_privod) + "\t";
    }
    float target;
    target = (position.length() <= 0) ? 0 : position.toFloat();
    for (int i = start; i < start + count; ++i)
    {
      privod[i].setTargetPositionFloat(target);
      privod[i].setEna(TRUE);
      privod[i].setRps(0.02);
      privod[i].setLastPulTimeUs();
    }
    response += String("toTarget=") + String(target);
  }
  return 0;
}

int doROTATE(String &param, String &response)
{
  String position, privod_str;
  int num_privod = 0, start, count;
  response = "";
  _EVS_CMDPARSERCLASS::splitCmdParam(param, position, privod_str);
  if (privod_str.length() <= 0)
    num_privod = NUM_PRIVOD + 1;
  else
    num_privod = min(NUM_PRIVOD - 1, abs(privod_str.toInt()));
  if (param == "?")
  {
    streamDoPeriodically(response);
  }
  else
  {
    response = String("ROTATE: privod=");
    if (num_privod > NUM_PRIVOD)
    {
      start = 0;
      count = NUM_PRIVOD;
      response += "ALL\t";
    }
    else
    {
      start = num_privod;
      count = 1;
      response += String(num_privod) + "\t";
    }
    float rotate;
    rotate = (position.length() <= 0) ? 0 : position.toFloat();
    for (int i = start; i < start + count; ++i)
    {

      privod[i].setTargetPositionFloat(privod[i].getCurrentPositionFloat() + rotate);
      privod[i].setEna(TRUE);
      privod[i].setRps(0.02);
      privod[i].setLastPulTimeUs();
    }
    response += String("toRotate=") + String(rotate);
  }
  return 0;
}

int doSTA(String &param, String &response)
{
  streamDoPeriodically(response);
  return 0;
}

int doIDLE(String &param, String &response)
{
  _isIdleResponse = !_isIdleResponse;
  response = String("IDLE=") + (_isIdleResponse ? "ON" : "OFF");
  return 0;
}
int doIDN(String &param, String &response)
{
  response = String(_VERSION_);
  return 0;
}

int doMOT(String &str, String &response)
{
  String cmd, param;
  int num_motor;
  _EVS_CMDPARSERCLASS::splitCmdParam(str, cmd, param);
  _checkCMD(cmd, "ABORT", doABORT, param, response);
  _checkCMD(cmd, "RENEW", doRENEW, param, response);
  _checkCMD(cmd, "ROTATE", doROTATE, param, response);
  _checkCMD(cmd, "GOTO", doGOTO, param, response);
  response = "UNKNOWN=\"MOT:" + str + "\"";
  return -1;
  /* _checkCMD(cmd, "HLP?", doMOTHLP, param, response);
    _checkCMD(cmd, "TST?", doTST, param, response);
    _checkCMD(cmd, "ENA", doENA, param, response);
    _checkCMD(cmd, "ENAINV", doENAINV, param, response);
    _checkCMD(cmd, "ENAAUTO", doENAAUTO, param, response);
    _checkCMD(cmd, "DIR", doDIR, param, response);
    _checkCMD(cmd, "RPS", doRPS, param, response);
    _checkCMD(cmd, "PUL", doPUL, param, response);
    _checkCMD(cmd, "PULWIDTH", doPULWIDTH, param, response);
    _checkCMD(cmd, "CONTMODE", doCONTMODE, param, response);
    _checkCMD(cmd, "TARGET", doTARGET, param, response);
    _checkCMD(cmd, "COUNT", doCOUNT, param, response);
    _checkCMD(cmd, "STEPPERREV", doSTEPPERREV, param, response);
    _checkCMD(cmd, "MICROSTEP", doMICROSTEP, param, response);
    _checkCMD(cmd, "SWITCH", doSWITCH, param, response);
    _checkCMD(cmd, "PARK", doPARK, param, response);
  */
}

int doRootCmd(String str, String &response)
{
  String cmd, param;
  _EVS_CMDPARSERCLASS::splitCmdParam(str, cmd, param);

  _checkCMD(cmd, "*IDN?", doIDN, param, response);
  _checkCMD(cmd, "*RST", doRESET, param, response);
  _checkCMD(cmd, "*STA?", doSTA, param, response);
  _checkCMD(cmd, "*???", doSTA, param, response);
  _checkCMD(cmd, "*IDLE", doIDLE, param, response);
  _checkCMD(cmd, "MOT", doMOT, param, response);
  response = "UNKNOWN=\"" + str + "\"";
  return -1;
}

int checkControl(void)
{
  int status;
  String response;
  status = parser.readStr();
  if (status >= 0)
  {
    //    Serial.println("STR: " + parser.getStr());
    status = doRootCmd(parser.getStr(), response);
    parser.sendResponse(status, response);
  }
  return status;
}

void privodPoll(void)
{
  for (int i = 0; i < NUM_PRIVOD; ++i)
  {
    if (privod[i].isTimeElapsed())
    {
      privod[i].pul();
    }
  }
}

void streamPoll()
{
  static long time = 0;
  checkControl();
  if (millis() > time)
  {
    if (_isIdleResponse)
    {
      String response;
      streamDoPeriodically(response);
      parser.sendResponse(0, response);
    }
    time = millis() + 2000;
  }
}

void initControl(void)
{
  SER_PORT.begin(115200);
  while (!SER_PORT)
    ;
  SER_PORT.setTimeout(3000);
  SER_PORT.println(_VERSION_);
  parser.setStreamIn(&SER_PORT);
  parser.setStreamOut(&SER_PORT);
  parser.setDelimiter(":");
}

void setup()
{
  int val;
  initControl();
  analogReference(EXTERNAL);

  setup_privod();
  _isIdleResponse = FALSE;
}

void loop()
{
  privodPoll();
  streamPoll();
}
