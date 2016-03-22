/*
(c) ElevenPaths 2016
https://github.com/latchdevel/crypto-iot

Forked from:
https://github.com/nuskunetworks/arduino_sha204 Copyright 2013 Nusku Networks

Change log:
	* limited to I2C protocol, SWI deleted
	* fix read error when available_bytes < count in uint8_t SHA204I2C::receive_bytes(uint8_t count, uint8_t *data);
	* add struct to store serial number
	* add selectable I2C address on init()
	* add cumston debug levels
	* add Progmen F() macro to debug strings
  * tune CPU_CLOCK_DEVIATION

To do:
	* fix unexpected alive response 

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

#ifndef SHA204_Library_h
#define SHA204_Library_h

#define SHA204_SERIAL_SIZE    ((uint8_t)  9)

class SHA204 {
private:
	virtual uint16_t SHA204_RESPONSE_TIMEOUT() = 0;
	void calculate_crc(uint8_t length, uint8_t *data, uint8_t *crc);
	uint8_t check_crc(uint8_t *response);
	virtual uint8_t receive_bytes(uint8_t count, uint8_t *buffer) = 0;
	virtual uint8_t send_bytes(uint8_t count, uint8_t *buffer) = 0;
	virtual uint8_t send_byte(uint8_t value) = 0;
	virtual uint8_t receive_response(uint8_t size, uint8_t *response) = 0;
	virtual uint8_t send_command(uint8_t count, uint8_t * command) = 0;
	virtual uint8_t chip_wakeup() = 0; // Called this because wakeup() was causing method lookup issues with wakeup(*response)

  virtual uint8_t DEBUG() = 0 ; // Add for set debug calls in SHA204 class

public:

  // ******** REVISAR ESTO QUE PARECE QUE NO SE USA
	uint8_t SHA204serialNumber[SHA204_SERIAL_SIZE] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,};

	virtual uint8_t sleep() = 0;
	uint8_t wakeup(uint8_t *response);

	uint8_t execute(uint8_t op_code, uint8_t param1, uint16_t param2,
			uint8_t datalen1, uint8_t *data1, uint8_t datalen2, uint8_t *data2, uint8_t datalen3, uint8_t *data3,
			uint8_t tx_size, uint8_t *tx_buffer, uint8_t rx_size, uint8_t *rx_buffer);
	uint8_t check_parameters(uint8_t op_code, uint8_t param1, uint16_t param2,
			uint8_t datalen1, uint8_t *data1, uint8_t datalen2, uint8_t *data2, uint8_t datalen3, uint8_t *data3,
			uint8_t tx_size, uint8_t *tx_buffer, uint8_t rx_size, uint8_t *rx_buffer);

	uint8_t send_and_receive(uint8_t *tx_buffer, uint8_t rx_size, uint8_t *rx_buffer, uint8_t execution_delay, uint8_t execution_timeout);
	virtual uint8_t resync(uint8_t size, uint8_t *response) = 0;

	uint8_t serialNumber(uint8_t *response);

	uint8_t check_mac(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t mode, uint8_t key_id, uint8_t *client_challenge, uint8_t *client_response, uint8_t *other_data);
	uint8_t derive_key(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t random, uint8_t target_key, uint8_t *mac);
	uint8_t dev_rev(uint8_t *tx_buffer, uint8_t *rx_buffer);
	uint8_t gen_dig(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t zone, uint8_t key_id, uint8_t *other_data);
	uint8_t hmac(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t mode, uint16_t key_id);
	uint8_t lock(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t zone, uint16_t summary);
	uint8_t mac(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t mode, uint16_t key_id, uint8_t *challenge);
	uint8_t nonce(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t mode, uint8_t *numin);
	uint8_t pause(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t selector);
	uint8_t random(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t mode);
	uint8_t read(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t zone, uint16_t address);
	uint8_t update_extra(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t mode, uint8_t new_value);
	uint8_t write(uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t zone, uint16_t address, uint8_t *value, uint8_t *mac);

  // ******** REVISAR ESTO
	virtual uint8_t reset_io() = 0;
	virtual uint8_t receive_byte(uint8_t *data) = 0;
};

#endif
