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


int payloadSector = 1; // which sector to read
int payloadBlock  = payloadSector * 4;

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
    int key[] = { 0xFF,  0xFF, 0xFF,0xFF, 0xFF, 0xFF};
    if(Rfid.authenticate(payloadBlock, 0xBB, key)) {
      Serial.println("authenticated successfully");
      // read the block:
     // Read the payload contained in this sector.
    String payload = Rfid.getNDEFpayload(payloadBlock);
    if (payload.length() > 0) {
      // Return the payload to the client
      Serial.print('U');
      Serial.print(payload); 
      
  
      }
      // print a final newline:
      Serial.println();
    }
  }
}


