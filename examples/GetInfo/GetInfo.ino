/*
 RFID Get Info
 
 Gets info from a SonMicro SM130 RFID reader
 
 Circuit:
 * Rfid  attached to pins A4 and A5 (SDA and SCL)
 
 created 29 May 2011
 by Tom Igoe
 
 This code is in the public domain
 */

#include <Wire.h>                // reader needs the Wire library
#include <SonMicroReader.h>

SonMicroReader Rfid;            // instance of the reader library

void setup() {
  // initialize serial:
  Serial.begin(9600);
  // initialize the reader:
  Rfid.begin();
  delay(2000);
  
  // try to read a tag:
  Rfid.selectTag();
 
  // print the last command used:
  Serial.print("Command: ");
  Serial.println( Rfid.getCommand(), HEX);

// print the packet length:
  Serial.print("packet length: ");
  Serial.println(Rfid.getPacketLength());

// print the tag number:
  Serial.print("tag number: ");
  Serial.println(Rfid.getTagNumber(), HEX);

// print the tag type:
  Serial.print("tag Type: ");
  Serial.println( Rfid.getTagType(), HEX);

// priont the error code and message, if there is one:
  Serial.print("error code: ");
  Serial.println( Rfid.getErrorCode(), HEX);
   
  // print the antenna power:
  Serial.print("Antenna power: ");
  Serial.println(Rfid.getAntennaPower());
 // print the firmware version:
  Serial.print("Firmware version: ");
  Serial.println(Rfid.getFirmwareVersion());
}

void loop() {
  // look for a tag:
  unsigned long tag = Rfid.selectTag();
  // if you get one, print its number and its type:
  if (tag != 0) {
    int tagType = Rfid.getTagType();
    Serial.print("Read a tag: ");
    Serial.println(tag, HEX);
    Serial.print("Tag type: ");
    Serial.println(tagType, HEX);
  }
  delay(1000);
}



