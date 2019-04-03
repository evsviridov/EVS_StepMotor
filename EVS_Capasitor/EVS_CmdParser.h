// __EVS_CMDPARSER_VER "0.1.4 from 05-Jul-2018"
#pragma once

#ifndef __EVS_CMDPARSER_H
#define __EVS_CMDPARSER_H

#include <Arduino.h>

#define SER_PORT Serial

#define _checkCMD( _cmd, _strVal, _func, _param, _response) {if ( (_cmd).equalsIgnoreCase((_strVal)) ) return _func((_param), (_response));}


#define _EVS_CMDPARSERCLASS EVS_CmdParser

class _EVS_CMDPARSERCLASS {
    String _str;
    String _cmd;
    String _param;
    String _delimiter = ":";
    Stream *_streamObjectIn;
    Stream *_streamObjectOut;
    //    HardwareSerial *_HWSerial = NULL;
  public:
    _EVS_CMDPARSERCLASS();
    _EVS_CMDPARSERCLASS(Stream *streamObjectInOut);
    _EVS_CMDPARSERCLASS(Stream *streamObjectIn, Stream *streamObjectOut);
    void setStreamIn(Stream *streamObjectIn);
    void setStreamOut(Stream *streamObjectOut);
    int readStr(void);
    int splitCmdParam(void);
    static int splitCmdParam(String &str, String &cmd, String &param, String delimiter=String(":"));
    int obtainCmd(void);
    String &getStr(void);
    String &getCmd(void);
    String &getParam(void);
    int sendResponse(int status, String &response);
    void dispResult(Stream *dispStream, char *header, String value);
    void setDelimiter(String delimiter = ":");
    String &getDelimiter(void) {return _delimiter;}
    String getErrorStr(int status);
};

#endif __EVS_CMDPARSER_H
