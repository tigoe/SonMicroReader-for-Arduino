/*
 RFID Read Block
 
 Reads block 4 of memory from a Mifare RFID tag
 using a SonMicro SM130 RFID reader.
 This assumes block 4 has been written to already,
 and is using the default authentication methiof
 
 Circuit:
 * SM130  attached to pins A4 and A5 (SDA and SCL)
 
 created 29 May 2011
 by Tom Igoe
 
 This code is in the public domain
 */

#include <Wire.h>                // reader needs the Wire library
#include <SonMicroReader.h>      

SonMicroReader Rfid;            // instance of the reader library

void setup() {
  // initalize serial communications and the reader:
  Serial.begin(9600); 
  Rfid.begin();
}


void loop() {
  // look for a tag:
  unsigned long tag = Rfid.selectTag();
  // if you get one, print its number and its type:
  if (tag != 0) {
    Serial.print("Read a tag: ");
    Serial.println(tag, HEX);
    // attempt to authenticate block 4:
    if(Rfid.authenticate(0x04)) {
      Serial.println("authenticated successfully");
      // read the block:
      Rfid.readBlock(0x04);
      // get the data as a string:
      String blockText = Rfid.getString();
      Serial.println(blockText);
      
     // get the data as an array of bytes:
      for(int i=0; i < 16; i++) {
        byte thisByte = Rfid.getPayload()[i];
        Serial.print(thisByte, HEX);
        Serial.print(" ");
      }
      // print a final newline:
      Serial.println();
    }
  }
}
