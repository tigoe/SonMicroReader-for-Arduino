/*
 SonMicroReader library for Arduino  
 
 Copyright (c) 2011 by Tom Igoe (tom.igoe@gmail.com)

This file is free software; you can redistribute it and/or modify
it under the terms of either the GNU General Public License version 2
or the GNU Lesser General Public License version 2.1, both as
published by the Free Software Foundation.

 
  Original library								(0.1) 

  Controls a SM130 RFID reader

*/


#include "SonMicroReader.h"
#include "Wire.h"

const boolean DEBUG = false;


SonMicroReader::SonMicroReader()
{
	command = 0;               		// received command, from the packet    
	packetLength = 0;          		// length of the response, from the packet
	checksum = 0;              		// checksum value received
	tagNumber = 0;    				// tag number 
	tagType = 0;               		// the type of tag
	errorCode = 0;             		// error code from some commands
	antennaPower = 1;          		// antenna power level
	version = "";
	payloadString.reserve(BLOCK_SIZE);
}


void SonMicroReader::begin(void)
{
  Wire.begin();
  reset();
  delay(2000);
}


void SonMicroReader::begin(int address)
{
  Wire.begin((uint8_t)address);		// allows user to send in I2C address
  reset();
}


// Retrieve a response from the reader
// from any command:

int SonMicroReader::getData() 
{   
  int count = 0;
  // The reader can't respond in less than
  // 50 ms:
  delay(50);
  // get response from reader:
  Wire.requestFrom(0x42, BUFFER_SIZE);
  while (!Wire.available()) if (DEBUG) Serial.print(".");
  // while data is coming from the reader,
  // add it to the response buffer:
  while(Wire.available())  {     
    responseBuffer[count] = Wire.read();  
    count++;
  }  
  // put a 0 in the last byte after the response:
  responseBuffer[count] = 0;

  // fill in the global variables:
  packetLength = responseBuffer[0];
  command = responseBuffer[1];
  checksum = responseBuffer[count-1];

  if (DEBUG) printBuffer(count);

  // if packet length is 2, you have only a command and an error code:
  if (packetLength < 3) {
    errorCode = responseBuffer[2];
    tagType = 0;
    tagNumber = 0;
  } 

  // some messages generate error codes.  Return them here.
  switch (command) {
    // reset produces nothing as of version 2.8 of the SM130 firmware:
  case 0x80:
    break;
    // firmware version only produces the firmware version:
  case 0x81: 
    errorCode = 0;
    tagType = 0;
    tagNumber = 0;
       // if you got a good payload, it's the version number
    // add the response to the string:
    if (packetLength > 2) {
      int i = 2;
      while (isPrintable(responseBuffer[i])) {
        version += (char)responseBuffer[i];
        i++;
      }
    }
    break;
  case 0x82:  // seekTag    
    if (errorCode == 0x55) {
      //Reader error: Antenna is off
    }
    if (errorCode == 0x4C) {
      //Command in Progress
    }
    // if you got a good payload, it's a tag number:
    if (packetLength > 2) {
      // get the tag type:
      tagType = responseBuffer[2];
      // tag bytes come in reverse order:
      for (int thisByte = packetLength; thisByte >= 3; thisByte--) {
        // shift the current byte up one byte:
        tagNumber = tagNumber << 8;
        // add the new byte to the end of the tag:
        tagNumber += responseBuffer[thisByte]; 
      }
    }
    break;
  case 0x83:  // selectTag
    switch(errorCode) {
    case 0x4E:
      // Reader error: No Tag present
      break;
    case 0x55:
      // Reader error: Access failed because RF field is off
      break;
    }
    // if you got a good payload, it's a tag number:
    if (packetLength > 2) {
      // get the tag type:
      tagType = responseBuffer[2];
      // tag bytes come in reverse order:
      for (int thisByte = packetLength; thisByte >= 3; thisByte--) {
        // shift the current byte up one byte:
        tagNumber = tagNumber << 8;
        // add the new byte to the end of the tag:
        tagNumber += responseBuffer[thisByte]; 
      }
    }
    break;
  case 0x85:  //authenticate
    switch(errorCode) {
    case 0x45:
      // Reader error: Invalid key format in EEPROM
      break;
    case 0x4C:
      // Login successful
      break;
    case 0x4E:
      // Reader error: No tag present, or login failed
      break;
    case 0x55:
      //Reader error: login failed
      break;
    }
    break;
  case 0x86:  //read block
    switch(errorCode) {
    case 00:
      // good read
      for (int i=0; i<count-1; i++) {
      	// payload starts at fourth byte of response:
      	payload[i] = responseBuffer[i+3];
      }
      break;
    case 0x4E:
      // Reader error: no tag present
      break;
    case 0x46:
      // Reader error: read failed
      break;
    }
    break; 
  case 0x87:  //read value block
    switch(errorCode) {
    case 0x4E:
      // Reader error: no tag present
      break;
    case 0x46:
      // Reader error: read failed
      break;
    case 0x49:
      // Reader error: you tried to read a block number that isn't a value block
      break;
    }
    break; 
  case 0x89:  //write  block
    switch(errorCode) {
    case 0x55:
      // Reader error: data read doesn't match data write
      break;
    case 0x58:
      // Reader error: the block you tried to write to is protected
      break;
    case 0x4E:
      // Reader error: no tag present
      break;
    case 0x46:
      // Reader error: write failed
      break;
    }
    break; 
  case 0x8A:  //write value block
    switch(errorCode) {
    case 0x4E:
      // Reader error: no tag present
      break;
    case 0x46:
      // Reader error: read failed during verification
      break;
    case 0x49:
      // Reader error: you tried to write to a block number that isn't a value block
      break;
    }
    break; 
  case 0x8B:  //write  4 byte block
    switch(errorCode) {
    case 0x55:
      // Reader error: data read doesn't match data write
      break;
    case 0x58:
      // Reader error: the block you tried to write to is protected
      break;
    case 0x4E:
      // Reader error: no tag present
      break;
    case 0x46:
      // Reader error: write failed
      break;
    }
    break; 
  case 0x8C:  //write master key
    switch(errorCode) {
    case 0x4E:
      // Reader error: write master key failed
      break;
    case 0x4C:
      // success!
      break;
    }
    break; 
  case 0x90:     // set antenna status
    errorCode = 0;
    antennaPower = responseBuffer[2];
    break;
  case 0x94:  //set baud rate
    switch(errorCode) {
    case 0x4E:
      // Reader error: set baud rate failed
      break;
    case 0x4C:
      // success!
      // Baud rate set";
      break;
    }
    break; 
  } 
  // return the length of the response:
  return count;
}


// This method sends a command to the 
// RFID readers via I2C. This function is 
// overloaded, meaning there are two versions, so you
// can call it in more than one way:

void SonMicroReader::sendCommand(int thisCommand) 
{
  byte length = 1;
  // convert the byte to an array:
  int commandBuffer[] = {
    thisCommand                  };
  // call the other sendCommand method:
  sendCommand(commandBuffer, length);
}

// Send a command to the reader
//

void SonMicroReader::sendCommand(int command[], int length) 
{
  Wire.beginTransmission(0x42); 
  int checksum = length;       // Starting value for the checksum.
  Wire.write(length);          // send the length

    for (int i = 0; i < length; i++) {
    checksum += command[i];    // Add each byte to the checksum
    Wire.write(command[i]);    // send the byte
  }

  // checksum is the low byte of the sum of 
  // the other bytes:
  checksum = checksum % 256; 
  Wire.write(checksum);        // send the checksum
  Wire.endTransmission();      // end the I2C connection

    // you just sent a new command, so there's no new data available:

  clearBuffer();
  clearValues();
}

//	return the last command sent
//

int SonMicroReader::getCommand() 
{
  return command;
}

//return  packetLength the length of the payload as reported by the reader
//

int SonMicroReader::getPacketLength() 
{
  return packetLength;
}


// return  checksum the checksum from the reader
// 

int SonMicroReader::getCheckSum() 
{
  return checksum;
}


// return  tagNumber the tag number
//
 
unsigned long SonMicroReader::getTagNumber() 
{
  return tagNumber;
}


/**
 * Assuming there's a valid tag read, this returns the tag type, as follows:
 *
 * 0x01 Mifare Ultralight 
 * 0x02 Mifare Standard 1K 
 * 0x03 Mifare Classic 4K 
 * 0xFF Unknown Tag type 
 *
 * @return  tagType an integer for the tag type
 * 
*/
 
int SonMicroReader::getTagType() 
{
  return tagType;
}


//	return the error code:
//

int SonMicroReader::getErrorCode() 
{
  return errorCode;
}

/**
 * 
 * @return  antennaPower    the antenna power.  0x00 is off, anything else is on
 */
 
int SonMicroReader::getAntennaPower() 
{
  return antennaPower;
}

//	reset the reader
//

void SonMicroReader::reset() 
{
  sendCommand(SM13X_RESET);
  // reset gets no response as of I2C version 2.8
}

/**
 * gets the firmware of the reader
 *
 */
String SonMicroReader::getFirmwareVersion() 
{
  sendCommand(SM13X_GET_FIRMWARE);
  // wait for a response:
  getData();
  return version;
}

/**
 * Sends the Seek Tag command 
 *
 */
 
void SonMicroReader::seekTag() 
{
  sendCommand(SM13X_SEEK);
  getData();
}


/**
 * Sends the Select Tag command 
 *
 */
unsigned long SonMicroReader::selectTag() 
{
  sendCommand(SM13X_SELECT);
  getData();
  return tagNumber;
}



// Authenticate yourself to the RFID tag.
//
boolean SonMicroReader::authenticate(int thisBlock) 
{
  int length = 3;
  int command[] = {
    SM13X_AUTHENTICATE,  // authenticate
    thisBlock,
    0xFF  		  // default encryption
  };  
  int key[] = {0};
  return authenticate(thisBlock, 0xFF, key);
}


boolean SonMicroReader::authenticate(int thisBlock, int authentication) 
{
  int key[] = {0};
   return authenticate(thisBlock, authentication, key);
}

boolean SonMicroReader::authenticate(int thisBlock, int authentication, int* thisKey) 
{
  int length = 9;
  int command[length];
  command[0] = SM13X_AUTHENTICATE,  // authenticate
  command[1] = thisBlock;
  command[2] = authentication;
  
  for (int i=0; i<length-3; i++) {
    command[i+3] = thisKey[i];
  }
  // send the command:
  sendCommand(command, length);

  // wait for a response:
  getData();

  // response 0x4C (ASCII L) means
  // you successfully authenticated
  if (responseBuffer[2] == 0x4C) {
    return true;
  } 
  else {
    // No tag or login failed
    return false;
  }
}



// Read a block. You need to authenticate() 
 // before you can call this.
  
 int SonMicroReader::readBlock(int block) 
 {
	 int length = 2;
	 int command[] = {
	 	SM13X_READ,  // read block
	 	block 
	 };
	 // send the command:
	 sendCommand(command, length);  
	 // get 20 bytes (3 response + 16 bytes data + checksum)
	 int count = getData();  
	 // response 0x4E (ASCII N) means no tag:
	 if (responseBuffer[2] == 0x4E) { 
	 	return 0;
	 } 
	 // response 0x46 (ASCII F) means read failed:
	 else if (responseBuffer[2] == 0x46) {
	 	return 0;
	 }
	 
		
	 return count;
 }
 
 
//	returns the read block as a String
//

String& SonMicroReader::getString()
{
	 payloadString = "";
	for(int i=0; i < 16; i++) {
		char thisChar = payload[i];
		if (thisChar !=0) {
			payloadString += thisChar;      
		}
	}

	return payloadString;
}

// Write block.  Not implemented yet.  Still need to 
// convert from Java 
//

void SonMicroReader::writeBlock(int thisBlock, String thisMessage) 
{
  //  // block needs to be broken into 16-byte sections
  //  if (thisMessage.length() > 16) {
  //    // You can't send more than 16 bytes with a writeBlock() command.";
  //  } 
  //  else {
  //    int thisCommand[] = {
  //      0x89,thisBlock                                                                                   }; 
  //    int thisByte;
  //    // make sure to write to all 16 bytes:
  //    for (int i = 0; i < 16; i++) {
  //      if (i < thisMessage.length()) {
  //        thisByte = (int)thisMessage.charAt(i);
  //      } 
  //      else {
  //        thisByte = 0;
  //      }
  //      thisCommand = parent.append(thisCommand, thisByte);
  //    } 
  //
  //    sendCommand(thisCommand);
  //  }
}

/**
 * Writes a 4-byte string to a block. If the string is less than 16 bytes
 * it fills the empty bytes with 0x00.  If more than 4 bytes, it generates
 * an error message.
 * Used for Mifare ultralight tags
 * 
 * You need to select and authenticate before ou can read or write.
 * 
 * @param thisBlock     Block to write to
 * @param thisMessage   4-byte string to write

  Not implemented yet.  Still need to convert from Java 

 */
 
void SonMicroReader::writeFourByteBlock(int thisBlock, String thisMessage) 
{
  //  // block needs to be broken into 4-byte sections
  //  if (thisMessage.length() > 4) {
  //    // You can't send more than 4 bytes with a writeFourByteBlock() command.";
  //  } 
  //  else {
  //    int thisCommand[] = {
  //      0x8B,thisBlock                                                                                   }; 
  //    int thisByte;
  //    // make sure to write to all 4 bytes:
  //    for (int i = 0; i < 4; i++) {
  //      if (i < thisMessage.length()) {
  //        thisByte = (int)thisMessage.charAt(i);
  //      } 
  //      else {
  //        thisByte = 0;
  //      }
  //      thisCommand = parent.append(thisCommand, thisByte);
  //    } 
  //    sendCommand(thisCommand);
  //  }
}

/**
 * Sets the antenna power.  0x00 is off, anything else is on
 * @param level the antenna power level
 */
void SonMicroReader::setAntennaPower(int level) 
{
  int thisCommand[] = {
    SM13X_SET_ANTENNA_POWER, level};
  sendCommand(thisCommand, 2);
  getData();
}

/**
 * Sends the sleep command (0x96)
 *
 */
void SonMicroReader::sleep() {
  sendCommand(SM13X_SLEEP);
  getData();
}

/**
 * Sets the UART baud rate
 *
 */

void SonMicroReader::setBaudRate(int baudRate) 
{

  int dataRate = 00;

  switch (baudRate){
  case 9600:
    dataRate = 0x00;
    break;
  case 19200:
    dataRate = 0x01;
    break;
  case 38400:
    dataRate = 0x02;
    break;
  case 57600:
    dataRate = 0x03;
    break;
  case 115200:
    dataRate = 0x04;
    break;
  }
  int thisCommand[] = { 
    SM13X_SET_BAUDRATE, dataRate}; 
  sendCommand(thisCommand, 2);
  // wait for a response:
  getData();
}

//	Clears the response buffer
//

void SonMicroReader::clearBuffer() 
{
  for (int i = 0; i < BUFFER_SIZE; i++) {
    responseBuffer[i] = 0;
  } 
}

// clears the class variables
//

void SonMicroReader::clearValues()
{
	packetLength = 0;          		// length of the response, from the packet
	checksum = 0;              		// checksum value received
	tagNumber = 0;    				// tag number 
	tagType = 0;               		// the type of tag
	errorCode = 0;             		// error code from some commands
	// ";          		// descriptive error message
}


// for debugging only: prints the response buffer
//

void SonMicroReader::printBuffer(int count) 
{
  for (int i = 0; i < count; i++) {
    if (DEBUG) Serial.print(responseBuffer[i], HEX);
    if (DEBUG) Serial.print(" ");
  } 
  if (DEBUG) Serial.println();
}


String SonMicroReader::getNDEFpayload(int startBlock, int authentication, int* thisKey) {

  int length = 0;
  String thisString = "";

  int startByte = 0;
  int currentBlock = startBlock;
  if (!authenticate(currentBlock, authentication, thisKey)) {
    thisString = "Error: could not authenticate against block ";
    thisString += int(currentBlock);
    return thisString;
  }
    
  // Read the first block and figure out what kind of payload we have.
  int readSize = readBlock(currentBlock);
  if (readSize != 24) {
    
    thisString = "Error: expected 24 bytes, got: ";
    thisString += (int) readSize;
    return thisString;
    
  } else {
    
    switch (responseBuffer[10]) {
      case 0x55: // URI data type
        startByte = 12;  // offset of payload in first block response   
        length = responseBuffer[9];   
        break;
      
      case 0x54: // text data type
        if (responseBuffer[11] == 0x02) { // UTF-8 + two-byte language code
          startByte = 14; // 12 + 2 bytes for language code
          length = responseBuffer[9] - 2;
        } else {
          thisString = "Error: unsupported character set: ";
          thisString += (int) responseBuffer[11];
          return thisString;
        }
        break;
      
      default:
        thisString = "Error: unknown record type: ";
        thisString += (int) responseBuffer[10];
        return thisString;
    }
  }
  
  // Read in each block and pull out the payload.
  //
  while(length > 0) {
      
    for (int j = startByte; j < 19; j++) {
      if (--length > 0) { // Keep adding characters until we reach the length.
        thisString += (char) responseBuffer[j];
      }
    }
    
    // Move on to the next block
    currentBlock++;
    if ((currentBlock + 1) % 4 == 0) { // Skip security blocks
      currentBlock++;
        
      // If we cross a sector boundary, we need to authenticate
      // against its first block.
      //
      if (!authenticate(currentBlock, authentication, thisKey)) {
        thisString = "Error: could not authenticate against sector: ";
        thisString += (int) currentBlock;
        return thisString;
      }
    }
    
    // Read the block.
    readBlock(currentBlock);
    startByte = 3;  // offset of payload in next subsequent block responses
  }
  return thisString;
}







