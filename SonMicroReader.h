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

// ensure this library description is only included once
#ifndef SonMicroReader_h
#define SonMicroReader_h

// include types & constants of core API (for Arduino after 0022)
#include "Arduino.h"
#include "Wire.h"

#define BUFFER_SIZE 24
#define BLOCK_SIZE 16

// commands for RFID reader:
#define SM13X_RESET 0x80  
#define SM13X_GET_FIRMWARE 0x81    
#define SM13X_SEEK 0x82
#define SM13X_SELECT 0x83
#define SM13X_AUTHENTICATE 0x85
#define SM13X_READ 0x86
#define SM13X_SET_ANTENNA_POWER 0x90
#define SM13X_SET_BAUDRATE 0x94
#define SM13X_SLEEP 0x96



// library interface description
class SonMicroReader 
{
  public:
	// constructors:   
	SonMicroReader();
	
	// public methods:
	void begin(void);					// initializes the reader and sends reset()
	void begin(int address);			// allows user to send in I2C address
	void sendCommand(int thisCommand);	// sends commands to reader
	void sendCommand(int command[], int length);	
	int getCommand();					// the value of the last command sent
	int getPacketLength();				// the length of the last packet received
	int getCheckSum();					// the checksum of the last packet received
	char* getPayload() {return payload;};	// the payload of the last packet
	String getString();						// the payload as a String
	unsigned long getTagNumber();			// the last tag number read
	int getTagType();						// the last tag type (see SM130 datasheet sec. 5.3)
	int getErrorCode();						// the error code last returned (see datasheet)
	int getAntennaPower();					// the antenna power (0 or 1)
	void reset();							// resets the unit
	String getFirmwareVersion();			// returns the firmware version
	void seekTag();							// starts a seek command
	unsigned long selectTag();				// starts a select command
	boolean authenticate(int thisBlock);						// authenticates using default auth
	boolean authenticate(int thisBlock, int authentication);	// authenticates using default key	
	boolean authenticate(int thisBlock, int authentication, int thisKey[]);	// custom auth
	int readBlock(int block);								// reads a block  (must auth first)
	void writeBlock(int thisBlock, String thisMessage);		// writes a block (must auth first)
	void writeFourByteBlock(int thisBlock, String thisMessage);	// writes 4-byte block (must auth first)
	void setAntennaPower(int level);		// sets antenna power
	void sleep();							// puts unit to sleep
	void setBaudRate(int baudRate);			// sets serial baud rate
	
private:
	 int command;               		// received command, from the packet    
	 int packetLength;          		// length of the response, from the packet
	 int checksum;              		// checksum value received
	 unsigned long tagNumber;   		// tag number 
	 int tagType;               		// the type of tag
	 int errorCode;             		// error code from some commands
	 String version;          			// the firmware version
	 int antennaPower;          		// antenna power level
	 byte responseBuffer[BUFFER_SIZE];	// To hold the last response from the reader
	 char payload[BLOCK_SIZE];			// payload for read and write blocks		
		
	int getData();						// waits for response from the reader
	void clearBuffer();					// clears response buffer
	void clearValues();					// clears private variables
	void printBuffer(int count);		// for debugging only; prints buffer

 };

#endif

