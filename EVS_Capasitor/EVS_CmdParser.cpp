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
  _streamObjectOut->println(response + "\tERR=" + status);
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
