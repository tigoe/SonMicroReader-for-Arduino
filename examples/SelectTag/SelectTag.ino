/*
 RFID Select Tag
 
 selects and reads a Mifare RFID tag's number
 using a SonMicro Rfid RFID reader, and compares it
 to the previous tag read.
 
 Circuit:
 * Rfid  attached to pins A4 and A5 (SDA and SCL)
 
 created 29 May 2011
 by Tom Igoe
 
 This code is in the public domain
 */

#include <Wire.h>                // reader needs the Wire library
#include <SonMicroReader.h>

SonMicroReader Rfid;            // instance of the reader library

unsigned long lastTag = 0;      // previous tag read

void setup() {
 // initalize serial communications and the reader:
  Serial.begin(9600); 
  Rfid.begin();
}

void loop() {
  // if selectTag reported a good tag number,
  // you'll have a number other than 0:
  unsigned long tag =  Rfid.selectTag();    
  
  if (tag != 0) {
    Serial.println(tag, HEX);
    if (tag == lastTag) {
      Serial.println("Same tag as last read");
    } 
    else {
      Serial.println("new tag"); 
    }
    // save this tag to compare with the next:
    lastTag = tag;
  } 
}
