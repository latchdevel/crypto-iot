/*
(c) ElevenPaths 2016
https://github.com/latchdevel/crypto-iot
*/

#include <SHA204.h>
#include <SHA204Definitions.h>

#include <Wire.h>
#include <SHA204I2C.h>

#ifndef TwoWire_h
  #error "Only start I2C if Wire.h is included"
#endif

#define SHA204I2C_ADDRESS  0x64
#define SHA204I2C_DEBUG    DEBUG_TRACE

SHA204I2C sha204dev(SHA204I2C_ADDRESS); // if emply set to default 0x64

uint8_t SHA204serialNumber[SHA204_SERIAL_SIZE];
uint8_t SHA204response = 0;
uint8_t tx_buffer[40];
uint8_t rx_buffer[SHA204_RSP_SIZE_MAX]; 
uint8_t numin32[32] = {};
uint8_t sha1[20] = {};   

void setup() {
  
  Serial.begin(115200);
  while (!Serial);

  Serial.println("\nBEGIN!\n");
  Serial.print("File: "); Serial.println(__FILE__);
  Serial.print("Date: "); Serial.print(__DATE__);Serial.print(" ");Serial.print(__TIME__);
  Serial.print(" Compiler: "); Serial.println(__cplusplus);
  Serial.println();

  Serial.println("Starting I2C");
  Wire.begin();
  
  Serial.println("Starting device");
  sha204dev.init(SHA204I2C_DEBUG); // Be sure to wake up device right as I2C goes up otherwise you'll have NACK issues
                                   // Call chip_wakeup(); and delay()


    Serial.println("\nSerial Number: ");
    SHA204response = SHAgetserial(sha204dev,SHA204serialNumber);
    if (SHA204response != 0){
      Serial.print("Error getting serial number 0x");
      Serial.println(SHA204response,HEX);
      delay(5000);
      SHA204response = SHAgetserial(sha204dev,SHA204serialNumber);
    }

       Serial.print("Response = ");Serial.flush();
        for (int i=0; i<sizeof(SHA204serialNumber); i++) {
              if (SHA204serialNumber[i]<0x10) {Serial.print("0");}Serial.print(SHA204serialNumber[i], HEX);
        }
        Serial.println();Serial.flush();  

    //Serial Number should be like "0x01 0x23 0xXX 0xXX 0xXX 0xXX 0xXX 0xXX 0xEE");
    Serial.print("Serial number is ");
    if (SHAserialok(SHA204serialNumber)){
       Serial.println("OK");
    }else{
       Serial.println("ERROR!!");
    }

    Serial.flush();

       Serial.println("\nSHA1:");Serial.flush();
       Serial.print("Response = ");Serial.flush();
        for (int i=0; i<sizeof(sha1); i++) {
              if (sha1[i]<0x10) {Serial.print("0");}Serial.print(sha1[i], HEX);
        }
        Serial.println();Serial.flush(); 
   
    Serial.println("\nNonce:");
    SHA204response = sha204dev.nonce(tx_buffer, rx_buffer, 0, sha1);
    Serial.print("Response = ");Serial.flush();
    if (SHA204response != 0){
      Serial.print("Error number 0x");Serial.flush();
      Serial.println(SHA204response,HEX);Serial.flush();
    }else{
        for (int i=1; i<sizeof(rx_buffer)-2; i++) { 
              //Serial.print(" 0x");
              if (rx_buffer[i]<0x10) {Serial.print("0");}Serial.print(rx_buffer[i], HEX);
        }
        Serial.println();Serial.flush();  
    }

    Serial.println("\nHMAC:");
    SHA204response = sha204dev.hmac(tx_buffer, rx_buffer, 0, 2);
    Serial.print("Response = ");Serial.flush();
    if (SHA204response != 0){
      Serial.print("Error number 0x");Serial.flush();
      Serial.println(SHA204response,HEX);Serial.flush();
    }else{
        for (int i=1; i<sizeof(rx_buffer)-2; i++) { 
              if (rx_buffer[i]<0x10) {Serial.print("0");}Serial.print(rx_buffer[i], HEX);
        }
        Serial.println();Serial.flush();  
    }
  
  Serial.println("\nDone!");
  Serial.flush();
  while (true);
}

void loop() {
    while (true);
}


uint8_t SHAgetserial(SHA204I2C _device, uint8_t *buffer){

  return  _device.serialNumber(buffer);

}

boolean SHAserialok(uint8_t *buffer){
  
  return ! ( (buffer[0] ^ 0x01) | (buffer[1] ^ 0x23) | (buffer[8] ^ 0xEE));

}

