#line 1 "c:\\Users\\jack\\Documents\\Arduino\\EVS_Capasitor\\EVS_CmdParser.cpp"
#line 1 "c:\\Users\\jack\\Documents\\Arduino\\EVS_Capasitor\\EVS_CmdParser.cpp"
// __EVS_CMDPARSER_VER "0.1.0 from 14.06.2018"
#include <Arduino.h>
#include "EVS_CmdParser.h"

_EVS_CMDPARSERCLASS::_EVS_CMDPARSERCLASS()
{
}
_EVS_CMDPARSERCLASS::_EVS_CMDPARSERCLASS(Stream *streamObjectIn, Stream *streamObjectOut)
{
  //  _HWSerial = SerialPort;
  setStreamIn(streamObjectIn);
  setStreamOut(streamObjectOut);
}
_EVS_CMDPARSERCLASS::_EVS_CMDPARSERCLASS(Stream *streamObject)
{
  //  _HWSerial = SerialPort;
  setStreamIn(streamObject);
  setStreamOut(streamObject);
}

void _EVS_CMDPARSERCLASS::setStreamIn(Stream *streamObject)
{
  _streamObjectIn = streamObject;
}
void _EVS_CMDPARSERCLASS::setStreamOut(Stream *streamObject)
{
  _streamObjectOut = streamObject;
}

int _EVS_CMDPARSERCLASS::splitCmdParam(void)
{
  return splitCmdParam(_str, _cmd, _param, _delimiter);
}

int _EVS_CMDPARSERCLASS::splitCmdParam(String &str, String &cmd, String &param, String delimiter)
{
  int numField = -1;
  cmd = "";
  param = "";
  if (str.length() > 0)
  {
    int delimiter_pos = str.indexOf(delimiter);
    if (delimiter_pos >= 0)
    {
      cmd = str.substring(0, delimiter_pos);
      param = str.substring(delimiter_pos + delimiter.length());
      numField = 2;
    }
    else
    {
      cmd = str;
      param = "";
      numField = 1;
    }
  }
  return numField;
}

//template<typename SERIAL_TYPE>
//int _EVS_CMDPARSERCLASS::readCmd(SERIAL_TYPE &serPtr)
int _EVS_CMDPARSERCLASS::readStr(void)
{
  char s[30], symbol;
  int count=0;
  _str = "";
//  _streamObjectOut->println("wait...");
/*  
  if (_streamObjectIn->peek() < 0)
  {
    return -1;
  }
  _streamObjectOut->println("\n...read...");
*/
//#define CUSTOM_READLINE
#ifdef CUSTOM_READLINE
  count = 0;
  while (count < 19)
  {
    while (!_streamObjectIn->available())
      ;
    symbol = _streamObjectIn->read();
    _streamObjectOut->print((unsigned int)symbol);
    _streamObjectOut->println(String(" :") + String(symbol));

    if ((int)symbol == '\n')
      break;
    s[count] = symbol;
    count++;
  }
#else
  if (_streamObjectIn->available())
    count = _streamObjectIn->readBytesUntil('\n', s, 29);
//  count = Serial.readBytesUntil('\n', s, 19);
#endif CUSTOM_READLINE

  if (count <= 0)
    return -1;

  //	symbol=;
  if (s[count - 1] == '\r')
    --count;
  s[count] = '\x0';
  _str = String((char *)s);
  return count;
}

int _EVS_CMDPARSERCLASS::obtainCmd(void)
{
  readStr();
  return splitCmdParam();
}

int _EVS_CMDPARSERCLASS::sendResponse(int status, String &response)
{
  _streamObjectOut->println(response + "\t" + getErrorStr(status) + "\t" + status);
}
String &_EVS_CMDPARSERCLASS::getStr(void)
{
  return _str;
}
String &_EVS_CMDPARSERCLASS::getCmd(void)
{
  return _cmd;
}
String &_EVS_CMDPARSERCLASS::getParam(void)
{
  return _param;
}
void _EVS_CMDPARSERCLASS::setDelimiter(String delimiter)
{
  _delimiter = delimiter;
}

void _EVS_CMDPARSERCLASS::dispResult(Stream *dispStream, char *header, String value)
{
  dispStream->print(String(header) + "=");
  dispStream->println(value);
}
String _EVS_CMDPARSERCLASS::getErrorStr(int status)
{
  return String((status) ? "ERR" : "OK");
}

#line 1 "c:\\Users\\jack\\Documents\\Arduino\\EVS_Capasitor\\EVS_capasitor.ino"
#define _VERSION_ "EVS_Capasitor 0.1.0 from 16-Dec-2018"

#include <Arduino.h>
#include "EVS_CmdParser.h"
#include "EVS_voltmeter.h"
#include "EVS_motor.h"
#include "EVS_privod.h"

EVS_CmdParser parser;
#define MOT_CMD ("MOT")
const char *KAVYCHKA = "\"";
bool _isIdleResponse = FALSE;

#define MOT_MAXRPS 2
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

EVS_motor_pin_config pin_config[NUM_PRIVOD] = {pin0, pin1, pin2, pin3, pin4, pin5}; // ENA, PUL, DIR

int EVS_motor_analog_pin[NUM_PRIVOD] = {A0, A1, A2, A3, A4, A5}; //Potentiometer pin

class EVS_privod privod[NUM_PRIVOD];
EVS_motor_config motor_config[NUM_PRIVOD];
EVS_motor_pin_inversion pin_inversion[NUM_PRIVOD];
float value_percent[NUM_PRIVOD];

class HardwareSerial *stream = &Serial;

void streamDoPeriodically(String &response);

float rangeMap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float percent2rotate(float percent)
{
  return rangeMap(percent, 90, 10, 0, 40);
}

float rotate2percent(float rotate)
{
  return rangeMap(rotate, 0, 40, 90, 10);
}

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
    doVoltmeter(EVS_motor_analog_pin[i], &v, 200);
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

    rotate = percent2rotate(value_percent[i]);
    position = privod[i].computeRotateToPosition(rotate);
    privod[i].setCurrentPositionLong(position);
    s="RENEW motor="+String(i);
    s+="\tpercent="+String(value_percent[i],2);
    s+="\trotate="+String(rotate,2);
    s+="\tposition="+String(position);
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
  float percent = rotate2percent(rotate);
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
        p->setCurrentPositionFloat(percent2rotate(value));
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
    if (fabs(error) >= 0.5)
      p->setRps(p->getRps() + 0.02);
    else
      p->setRps(fmax(0.2, p->getRps() - 0.01));
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
    motor_config[i]._max_rps = MOT_MAXRPS;
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
    privod[i].setRps(1);
    privod[i].setPositionErrorRangeFloat(0.1, 0.3);
  }
  doSetupPositionAll();
}

void streamDoPeriodically(String &s)
{
  int i;
  bool current = 1, target = 1, count = 0;
  s = "";
  if (current)
  {
    s += "current=";
    for (i = 0; i < NUM_PRIVOD - 1; ++i)
    {
      s += String(privod[i].getCurrentPositionFloat(), 1);
      s += ";";
    }
    s += String(privod[i].getCurrentPositionFloat(), 1);
  }

  if (target)
  {
    s += "\ttarget=";
    for (i = 0; i < NUM_PRIVOD - 1; ++i)
    {
      s += String(privod[i].getTargetPositionFloat(), 1);
      s += ";";
    }
    s += String(privod[i].getTargetPositionFloat(), 1);
  }

  if (count)
  {
    s += "\tcount=";
    for (i = 0; i < NUM_PRIVOD - 1; ++i)
    {
      s += String(privod[i].getCounter());
      s += ";";
    }
    s += String(privod[i].getCounter());
  }
}

int doABORT(String &param, String &response)
{
  setup_privod();
  response = "ABORTed";
  return 0;
}

int doRESET(String &param, String &response)
{
  setup_privod();
  response = "RESETTed";
  return 0;
}
int doRENEW(String &param, String &response)
{
  doSetupPositionAll();
  response = "RENEWed";
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
    response = String("GOTOed privod=");
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
      privod[i].setRps(0.01);
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
    response = String("ROTATed privod=");
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
      privod[i].setRps(0.01);
    }
    response += String("toRotate=") + String(rotate);
  }
  return 0;
}

int doASK(String &param, String &response)
{
  streamDoPeriodically(response);
  return 0;
}

int doMOT(String &str, String &response)
{
  String cmd, param;
  int num_motor;
  _EVS_CMDPARSERCLASS::splitCmdParam(str, cmd, param);
  _checkCMD(cmd, "ABORT", doABORT, param, response);
  _checkCMD(cmd, "RESET", doRESET, param, response);
  _checkCMD(cmd, "RENEW", doRENEW, param, response);
  _checkCMD(cmd, "ROTATE", doROTATE, param, response);
  _checkCMD(cmd, "GOTO", doGOTO, param, response);
  _checkCMD(cmd, "?", doASK, param, response);
  response = str + "\tUNKNOWN:";
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

  // _checkCMD(cmd, "", doCOUNT, param, response);
  // _checkCMD(cmd, "*IDN?", doIDN, param, response);
  // _checkCMD(cmd, "*HLP?", doHLP, param, response);
  _checkCMD(cmd, MOT_CMD, doMOT, param, response);

  response = "\tUNKNOWN:" + str;
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
  checkControl();
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
  _isIdleResponse = TRUE;

}

void loop()
{
  privodPoll();
  streamPoll();
}

