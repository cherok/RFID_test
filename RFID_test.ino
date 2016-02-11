#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

// #define _DEBUG

// RX and TX for UNO
int SSrx = 8;
int SStx = 9;

// number of tags read since execution
int tagReadCount = 0;

// setup lcd
int lcdRS = 12;
int lcdEnable = 11;
int lcdD4 = 5;
int lcdD5 = 4;
int lcdD6 = 3;
int lcdD7 = 2;

// initialize LCD
LiquidCrystal lcd(lcdRS, lcdEnable, lcdD4, lcdD5, lcdD6, lcdD7);

// initialize serial for reader
SoftwareSerial RFIDReader(SSrx, SStx);

void setup()
{
  RFIDReader.begin(9600);
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.print("Scan `da Chicken");
}

void loop()
{ 
  // make sure we have enough to read...
  // too little and we might read from an empty
  // buffer
  if (RFIDReader.available() > 13)
  {
    String tag = getTag(&RFIDReader); 
    
    Serial.println(tag);     
    drawLCD(tag);
  }
}

void drawLCD(String _tag)
{
  lcd.setCursor(0, 0);
  lcd.print(_tag);

  lcd.setCursor(0, 1);
  lcd.print(tagReadCount);
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
    Serial.println(_tag);
  #endif

  return _tag;
}
