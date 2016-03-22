/*
(c) ElevenPaths 2016
https://github.com/latchdevel/crypto-iot

Forked from:
https://github.com/nuskunetworks/arduino_sha204 Copyright 2013 Nusku Networks

Change log:
	* see SHA204.h

	Licensed to the Apache Software Foundation (ASF) under one
	or more contributor license agreements. See the NOTICE file
	istributed with this work for additional information
	regarding copyright ownership. The ASF licenses this file
	to you under the Apache License, Version 2.0 (the
	"License"); you may not use this file except in compliance
	with the License. You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing,
	software distributed under the License is distributed on an
	"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
	KIND, either express or implied. See the License for the
	specific language governing permissions and limitations
	under the License.
*/

#include "Arduino.h"
#include <Wire.h>

#ifdef TwoWire_h // Esure this code only gets built if you have Wire.h included in the main sketch

#include "SHA204.h"
#include "SHA204ReturnCodes.h"
#include "SHA204Definitions.h"
#include "SHA204I2C.h"

uint16_t SHA204I2C::SHA204_RESPONSE_TIMEOUT() {
	return SHA204_RESPONSE_TIMEOUT_VALUE;
}

uint8_t SHA204I2C::DEBUG() {
	return DEBUG_LEVEL;
}

SHA204I2C::SHA204I2C() {
	address = ((uint8_t) 0x64);
}

SHA204I2C::SHA204I2C(uint8_t deviceAddress) {
	address = deviceAddress;
}

void SHA204I2C::init() {
	DEBUG_LEVEL = DEBUG_NO_LOG;
	chip_wakeup();
	delay(1);
}

void SHA204I2C::init(uint8_t _DEBUG_LEVEL) {
	DEBUG_LEVEL = _DEBUG_LEVEL;
	if (DEBUG()>=DEBUG_INFO) {Serial.println(F("* DEBUG * init()")); Serial.flush();}
	if (DEBUG()>=DEBUG_INFO) {Serial.print(F("* DEBUG * DEBUG_LEVEL = "));Serial.println(DEBUG_LEVEL,HEX);Serial.flush();}
	chip_wakeup();
	delay(1);
}

uint8_t SHA204I2C::receive_bytes(uint8_t count, uint8_t *data) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * receive_bytes()")); Serial.flush();}
	if (DEBUG()>=DEBUG_TRACE) {Serial.print(F("* DEBUG * count = "));Serial.println(count);Serial.flush();}
	uint8_t i;

	int available_bytes = Wire.requestFrom(deviceAddress(), count);
	if (DEBUG()>=DEBUG_TRACE) {Serial.print(F("* DEBUG * available_bytes = "));Serial.println(available_bytes);Serial.flush();}

	if (available_bytes > count) {
		if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * receive_bytes() FAIL available_bytes > count, available_bytes = "));Serial.println(available_bytes); Serial.flush();}
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}

  for (i = 0; i < available_bytes; i++) {
	//for (i = 0; i < count; i++) {
		while (!Wire.available()); // Wait for byte that is going to be read next
		*data++ = Wire.read(); // Store read value
	}

  if (available_bytes < count) {
		if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * receive_bytes() WARNING available_bytes < count, available_bytes = "));Serial.println(available_bytes); Serial.flush();}
		reset_io();
		if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * receive_bytes() TRY to read bytes = "));Serial.println(count-available_bytes); Serial.flush();}
			uint8_t d;
			uint8_t *pd;
			pd = &d;
			for (i=0; i <(count-available_bytes); i++){
				d = 0xFF;
				receive_byte(pd);
				if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * receive_bytes() TRY receive_byte() = 0x"));Serial.println(d,HEX); Serial.flush();}
				*data++ = *pd;
			}
   }

	return I2C_FUNCTION_RETCODE_SUCCESS;
}

uint8_t SHA204I2C::receive_byte(uint8_t *data) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * receive_byte()")); Serial.flush();}

	int available_bytes = Wire.requestFrom(deviceAddress(), (uint8_t)1);

	if (available_bytes != 1) {
		if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * receive_byte() FAIL available_bytes!=1, available_bytes = "));Serial.println(available_bytes); Serial.flush();}
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}
	while (!Wire.available()); // Wait for byte that is going to be read next
	*data++ = Wire.read(); // Store read value

	return I2C_FUNCTION_RETCODE_SUCCESS;
}

uint8_t SHA204I2C::send_byte(uint8_t value) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * send_byte()")); Serial.flush();}
	return send_bytes(1, &value);
}

uint8_t SHA204I2C::send_bytes(uint8_t count, uint8_t *data) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * send_bytes()")); Serial.flush();}

	int sent_bytes = Wire.write(data, count);

	if (count > 0 && sent_bytes == count) {
		return I2C_FUNCTION_RETCODE_SUCCESS;
	}
  if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * send_bytes() FAIL sent!=count, sent_bytes = "));Serial.println(sent_bytes); Serial.flush();}
	return I2C_FUNCTION_RETCODE_COMM_FAIL;
}

int SHA204I2C::start_operation(uint8_t readWrite) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * start_operation()")); Serial.flush();}
	int written = Wire.write(&readWrite, (uint8_t)1);

	return written > 0;
}

uint8_t SHA204I2C::chip_wakeup() {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * chip_wakeup()")); Serial.flush();}
	// This was the only way short of manually adjusting the SDA pin to wake up the device
	Wire.beginTransmission(deviceAddress());
	int i2c_status = Wire.endTransmission();
	if (i2c_status != 0) {
		if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * chip_wakeup() FAIL i2c_status = "));Serial.println(i2c_status,HEX); Serial.flush();}
		return SHA204_COMM_FAIL;
	}

	return SHA204_SUCCESS;
}

uint8_t SHA204I2C::receive_response(uint8_t size, uint8_t *response) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * receive_response()")); Serial.flush();}
	if (DEBUG()>=DEBUG_TRACE) {Serial.print(F("* DEBUG * size = "));Serial.println(size);Serial.flush();}
	uint8_t count;
	uint8_t i2c_status;

	// Receive count byte.
	i2c_status = receive_byte(response);
	if (DEBUG()>=DEBUG_TRACE) {Serial.print(F("* DEBUG * receive_response() receive count byte i2c_status = "));Serial.println(i2c_status);Serial.flush();}
	if (DEBUG()>=DEBUG_TRACE) {Serial.print(F("* DEBUG * receive_response() count byte content = "));Serial.println(response[0]);Serial.flush();}

	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS) {
		if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * receive_response() FAIL 1, i2c_status{1} = "));Serial.println(i2c_status,HEX);Serial.flush();}
		return SHA204_COMM_FAIL;
	}

  //#define SHA204_BUFFER_POS_COUNT      (0)  //!< buffer index of count byte in command or response
	//#define SHA204_BUFFER_POS_STATUS     (1)  //! buffer index of status byte in status response
	//#define SHA204_BUFFER_POS_DATA       (1)  //! buffer index of first data byte in data response

	count = response[SHA204_BUFFER_POS_COUNT];
	if ((count < SHA204_RSP_SIZE_MIN) || (count > size)) {
		if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * receive_response() FAIL 2, read count size invalid, count = "));Serial.println(count);Serial.flush();}
		if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * size = "));Serial.println(size);Serial.flush();}
		return SHA204_INVALID_SIZE;
	}

	// ESTO ES LO QUE FALLA. SE ESPERA LLER 35-1 = 34 bites, pero solo hay 32 disponibles.
	// 0x23 0x04 0x97 0x94 0xAF 0x4E 0x4F 0xB5 0xFF 0x93 0xDA 0x08 0xE2 0x82 0x33 0xBF 0xE7 0x0B 0xA5 0x3D 0x2E 0xC4 0xBC 0x03 0x93 0xC8 0xD7 0x13 0xD6 0x57 0x5A 0x3B 0x51 0x00 0x00
  //    0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   NL   NL

	i2c_status = receive_bytes(count - 1, &response[SHA204_BUFFER_POS_DATA]);

	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS) {
  if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * receive_response() FAIL 3, i2c_status{1} = "));Serial.println(i2c_status,HEX);Serial.flush();}
		return SHA204_COMM_FAIL;
	}

	return SHA204_SUCCESS;
}

uint8_t SHA204I2C::send(uint8_t word_address, uint8_t count, uint8_t *buffer) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * send()")); Serial.flush();}
	uint8_t i2c_status;

	Wire.beginTransmission(deviceAddress());

	start_operation(I2C_WRITE);

	i2c_status = send_bytes(1, &word_address);
	if (DEBUG()>=DEBUG_TRACE) {Serial.print(F("* DEBUG * i2c_status{1} = "));Serial.println(i2c_status,HEX);Serial.flush();}
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS) {
		 if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * send() fail 1, i2c_status{1} = "));Serial.println(i2c_status,HEX);Serial.flush();}
		return SHA204_COMM_FAIL;
	}

	if (count == 0) {
		return SHA204_SUCCESS;
	}

	i2c_status = send_bytes(count, buffer);
  if (DEBUG()>=DEBUG_TRACE) {Serial.print(F("* DEBUG * i2c_status{2} = "));Serial.println(i2c_status,HEX);Serial.flush();}
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS) {
		if (DEBUG()>=DEBUG_WARN)  {Serial.print(F("* DEBUG * send() fail 2, i2c_status{2} = "));Serial.println(i2c_status,HEX);Serial.flush();}
		return SHA204_COMM_FAIL;
	}

	Wire.endTransmission();

	return SHA204_SUCCESS;
}


uint8_t SHA204I2C::send_command(uint8_t count, uint8_t *command) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * send_command()")); Serial.flush();}
	if (DEBUG()>=DEBUG_TRACE) {
		Serial.print(F("* DEBUG * Sending "));Serial.print(count);Serial.print(F(" bytes:"));
		for (int i=0; i<count; i++) {
    	Serial.print(" 0x");
    	if (command[i]<0x10) {Serial.print("0");}
    	Serial.print(command[i], HEX);
  	}
  	Serial.println();
	}
	return send(SHA204_I2C_PACKET_FUNCTION_NORMAL, count, command);
}

uint8_t SHA204I2C::sleep(void) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * sleep()")); Serial.flush();}
	return send(SHA204_I2C_PACKET_FUNCTION_SLEEP, 0, NULL);
}

uint8_t SHA204I2C::resync(uint8_t size, uint8_t *response) {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * resync()")); Serial.flush();}

	// Try to re-synchronize without sending a Wake token
	// (step 1 of the re-synchronization process).
	uint8_t nine_clocks = 0xFF;
	send_bytes(1, &nine_clocks);
	Wire.beginTransmission(deviceAddress());
	Wire.endTransmission();

	// Try to send a Reset IO command if re-sync succeeded.
	int ret_code = reset_io();
	if (ret_code == SHA204_SUCCESS) {
		return ret_code;
	}

	// We lost communication. Send a Wake pulse and try
	// to receive a response (steps 2 and 3 of the
	// re-synchronization process).
	sleep();
	ret_code = wakeup(response);

//  if (DEBUG) {Serial.print("resync wakeup ret_code="); Serial.println(ret_code); Serial.flush();}
	// Translate a return value of success into one
	// that indicates that the device had to be woken up
	// and might have lost its TempKey.
	return (ret_code == SHA204_SUCCESS ? SHA204_RESYNC_WITH_WAKEUP : ret_code);
}

uint8_t SHA204I2C::reset_io() {
	if (DEBUG()>=DEBUG_INFO)  {Serial.println(F("* DEBUG * reset_io()")); Serial.flush();}
	return send(SHA204_I2C_PACKET_FUNCTION_RESET, 0, NULL);
}

#endif
