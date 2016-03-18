#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MySensor.h>
#include <Base64.h>  

#define _VERSION 0.2
#define  _NEST_BOX_ID 0

#define _DEBUG

// define RX and TX pins for UNO
#define _SSRX 8
#define _SSTX 7

// try to give some sort of unique ID
// for each device to inject into 
// the tag
#define _READER_ID "H3AD"

// number of tags read since execution
int tagReadCount = 0;

String readerID = "H3AD";

// initialize serial for reader
SoftwareSerial RFIDReader(_SSRX, _SSTX);

// setup the MySensor objects
MySensor gw;
MyMessage msg(S_CUSTOM, V_VAR1);

void setup()
{

  RFIDReader.begin(9600);
  Serial.begin(115200);

  // print version and reader identification
  Serial.println("NestMonitor Version: " + String(_VERSION));
  Serial.println("Reader ID: " + String(_READER_ID));

  Serial.println("Startup Radio");

  // startup wireless module
  gw.begin();
  Serial.print("Node ID: ");
  Serial.println(gw.getNodeId());
  // Send the sketch version information
  gw.sendSketchInfo("NestMonitor: ", "0.2");
  // register w/ the gateway
  gw.present(_NEST_BOX_ID, S_CUSTOM);
}

void loop()
{ 
  // make sure we have enough to read...
  // too little and we might read from an empty
  // buffer
  if (RFIDReader.available() > 13)
  {
    sendTag(getTag(&RFIDReader), msg, gw);         
  }
}

void sendTag(String _tag, MyMessage _msg, MySensor _gw)
{
  char _char_tag[_tag.length() + 1];
  _tag.toCharArray(_char_tag, sizeof(_char_tag));

  _gw.send(_msg.set(_char_tag));
}

String getTag(SoftwareSerial *_RFIDReader)
{
  String _tag;
  int _rfidvalue = _RFIDReader->read();

  // wait until we get a 2
  while(_rfidvalue != 2)
  {
    _rfidvalue = _RFIDReader->read();
  }

  if (_rfidvalue == 2)
  {
    _tag = _rfidvalue;

    for (int i = 0; i < 13; i++)
    {
      _tag += _RFIDReader->read();
    }
     ++tagReadCount;
  }
  _RFIDReader->flush();

  #ifdef _DEBUG
    Serial.println("DEBUG: " + _tag);
  #endif

  return String(trimTag(&_tag).toInt(), HEX);
}

String trimTag(String *_tag)
{
  String _trimmed_tag;

  for (int i = _tag->length() - 10; i < _tag->length() - 1; i++)
  {
    _trimmed_tag += _tag->charAt(i);
  }

  return _trimmed_tag;
}
