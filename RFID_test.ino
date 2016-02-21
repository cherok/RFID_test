#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

//#define _DEBUG

// define RX and TX pins for UNO
#define _SSRX 8
#define _SSTX 9

// try to give some sort of unique ID
// for each device to inject into 
// the tag
#define _READER_ID "H3AD"

// number of tags read since execution
int tagReadCount = 0;

// initialize serial for reader
SoftwareSerial RFIDReader(_SSRX, _SSTX);

void setup()
{
  RFIDReader.begin(9600);
  Serial.begin(9600);
}

void loop()
{ 
  // make sure we have enough to read...
  // too little and we might read from an empty
  // buffer
  if (RFIDReader.available() > 13)
  {
    String tag = getTag(&RFIDReader);
    
    Serial.println(String(_READER_ID) + "," + tag);     
  }
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

  return _tag;
}
