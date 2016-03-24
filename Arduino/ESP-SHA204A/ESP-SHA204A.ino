/*
(c) ElevenPaths 2016
https://github.com/latchdevel/crypto-iot
*/
// Wifi Libraries
#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager

// Atmel SHA204A libraries from   // https://github.com/latchdevel/crypto-iot/tree/master/Arduino/libraries
#include <SHA204.h>
#include <SHA204I2C.h>
#include <SHA204Definitions.h>

// Aux Libraries
#include <Hash.h>       // ESP8266 Arduino library to SHA1 only. Other platform can use Crytosuite from https://github.com/Cathedrow/Cryptosuite
#include <Base64.h>     // https://github.com/adamvr/arduino-base64   Copyright (C) 2013 Adam Rudd
#include <time.h>       // ESP-Arduino core tine library. Needed for time functions
#include <Ticker.h>     // For LED status. ESP8266 library that calls functions periodically

// Global defines
#define SHA204I2C_ADDRESS  0x64
#define SHA204I2C_DEBUG    DEBUG_NO_LOG
#define PIN_SDA            D4 // Default on ESP-12 NodeMCU
#define PIN_SCL            D5 // Default on ESP-12 NodeMCU
#define MY_BUTTON          D3 // FLASH_BUTTON on NodeMCU

// Global consts
const int8_t   TimeZone   =  0; // 0 for UTC        Central European Time (Winter)  (CET) Time zone offset: UTC +1 
const int8_t   TimeDST    =  0; // DaylightSavings  Central European Summer Time   (CEST) Time zone offset: UTC +2 March to Octuber
const char*    host       = "sweet11paths";
const char*    remoteHost = "sha204a.cf";
const uint16_t remotePort =  80;
const uint8_t  base[24]   ={ 0x11, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,  
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEE, 
                             0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0x00, 0x00 };
// Global vars
uint8_t SHA204serialNumber[SHA204_SERIAL_SIZE];
uint8_t SHA204response = 0;
uint8_t tx_buffer[32]; 
uint8_t rx_buffer[SHA204_RSP_SIZE_MAX];       // ((uint8_t) 35)  //!< maximum size of response packet
uint8_t  reqsha[20] = {};                             
uint8_t c_nonce[32] = {};
uint8_t  c_hmac[32] = {};

// Glbal objects
SHA204I2C     sha204dev(SHA204I2C_ADDRESS); // if emply set to default 0x64
Ticker        ticker;
time_t        t;
WiFiManager   wifiManager;
WiFiClient    HTTPSclient;

void tick(){ //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO16 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("CallBack! Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.3, tick);
}

uint8_t SHAgetserial(SHA204I2C _device, uint8_t *buffer){
  return  _device.serialNumber(buffer);
}

boolean SHAserialok(uint8_t *buffer){
  return ! ( (buffer[0] ^ 0x01) | (buffer[1] ^ 0x23) | (buffer[8] ^ 0xEE));
}

void setup() {
  
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);
    ticker.attach(0.1, tick);
    delay(1000);
    while(!Serial);
    Serial.println("\nBEGIN!");

// Print ESP device status  
// ********************************************************************************************************************* //
    Serial.println();
    Serial.print("File: "); Serial.println(__FILE__);
    Serial.print("Date: "); Serial.print(__DATE__);Serial.print(" ");Serial.print(__TIME__);
    Serial.print(" Compiler: "); Serial.println(__cplusplus);
    Serial.println();

    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();

    Serial.printf("Flash chip   id: 0x%08X\n", ESP.getFlashChipId());
    Serial.printf("Flash real size: %u bytes\n", realSize);
    Serial.printf("Flash ide  size: %u bytes\n", ideSize);
    Serial.printf("Flash ide speed: %u Hz\n", ESP.getFlashChipSpeed());
    Serial.printf("Flash ide  mode: %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

    if(ideSize != realSize) {
        Serial.println("Flash chip configuration wrong!\n");
    } else {
        Serial.println("Flash chip configuration ok.\n");
    }

    Serial.print("ESP id: 0x"); Serial.println(ESP.getChipId());
    Serial.printf("Free   heap:  %u bytes\n", ESP.getFreeHeap(), DEC);
    Serial.printf("Sketch size: %u bytes\n", ESP.getSketchSize());
    Serial.printf("Free   size: %u bytes\n", ESP.getFreeSketchSpace());
    Serial.println();
    
// Init I2C bus and get SHA204A device Serial Number   
// ********************************************************************************************************************* //
    Serial.println("Starting I2C");
    // Wire.begin(int sda, int scl) on ESP, else they default to pins 4 (SDA) and 5 (SCL).
    Wire.begin(PIN_SDA,PIN_SCL);
    Serial.println("Starting device SHA204A");Serial.flush();  
    sha204dev.init(SHA204I2C_DEBUG); // Be sure to wake up device right as I2C goes up otherwise you'll have NACK issues
                                     // Call chip_wakeup(); and delay()
    Serial.print("Getting Serial Number: ");Serial.flush();  
    SHA204response = SHAgetserial(sha204dev,SHA204serialNumber);
    if (SHA204response != 0){
        Serial.printf("Error 0x%02X\n", SHA204response);Serial.flush();
        memcpy(SHA204serialNumber,"ERRROR!!!",SHA204_SERIAL_SIZE);
    }else{
       for (int i=0; i<sizeof(SHA204serialNumber); i++) {
            Serial.printf("%02X", SHA204serialNumber[i]);
       }
       Serial.println();Serial.flush();  
    }
    Serial.print("Serial number is ");
    if (SHAserialok(SHA204serialNumber)){
       Serial.println("OK");
    }else{
       Serial.println("ERROR!!");
    }
    Serial.flush();

// Init wifi network
// ********************************************************************************************************************* //
  ticker.attach(0.6, tick);
  wifiManager.setDebugOutput(false);
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep in seconds
  wifiManager.setTimeout(180); 
  
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  Serial.print("\nWaiting for wifi");
  if(!wifiManager.autoConnect(host)) {
    Serial.println("\nFailed to connect and hit timeout. Rebooting....");
    delay(5000);
    ESP.reset(); //reset and try again, or maybe put it to deep sleep
  }

  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
  }
  Serial.println(" Done!");
  Serial.print("Local IP: "); Serial.println(WiFi.localIP());
  Serial.print("Net Mask: "); Serial.println(WiFi.subnetMask());
  Serial.print("Gateway.: "); Serial.println(WiFi.gatewayIP());  
  
// Get time from NPT server
// ********************************************************************************************************************* //  
  configTime(TimeZone * 3600, TimeDST, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" Done!");
  Serial.print("UTC Time = ");
  time_t now = time(nullptr);
  Serial.print(ctime(&now));

  while (WiFi.status() != WL_CONNECTED){
    delay(100);
  }
  Serial.println("ALL DONE!");Serial.flush();
  pinMode(MY_BUTTON, INPUT_PULLUP);
  ticker.attach(1.0, tick);
}

void loop() {
 //Check Wifi status
  if(WiFi.status() != WL_CONNECTED) {
    while (WiFi.status() != WL_CONNECTED){
      t = time(nullptr);
      Serial.print("Wifi lost status 0x0");
      Serial.print(WiFi.status(),HEX);
      Serial.print(" at ");
      Serial.print( ctime(&t) );
      delay(5000);
    }
    t = time(nullptr);
    Serial.print("Wifi restaured at ");Serial.print( ctime(&t) );
    Serial.print("STA MAC.: "); Serial.println(WiFi.macAddress());
    Serial.print("Local IP: "); Serial.println(WiFi.localIP());
    Serial.print("Net Mask: "); Serial.println(WiFi.subnetMask());
    Serial.print("Gateway.: "); Serial.println(WiFi.gatewayIP());   
  }

//Check button 
  if ( digitalRead(MY_BUTTON) == LOW ) {
    ticker.attach(0.1, tick);
    Serial.print("\nConnecting to http://");
    Serial.print(remoteHost);Serial.print(":");
    Serial.println(remotePort,DEC);
   
    if (!HTTPSclient.connect(remoteHost, remotePort)) {
      Serial.println("Connection failed");
    }else{
      Serial.println("Connection OK!");Serial.flush();

// Get current time and generate request string
// ********************************************************************************************************************* //
      t = time(nullptr);
      String timestamp = String(t,DEC);
      String method = "GET";
      String url="/?timestamp=" +timestamp;
      String request = method + " " + url + " HTTP/1.1\r\n";

      Serial.print("Request: ");
      Serial.println(request);

// Compute SHA1 digest from request string
// ********************************************************************************************************************* //

      sha1(request, &reqsha[0]);
    
      Serial.print("SHA1:  ");
      for(uint16_t i = 0; i < 20; i++) {
        Serial.printf("%02X", reqsha[i]);
      }
      Serial.println();Serial.flush();
    
// Wake up SHA204A
// ********************************************************************************************************************* //

      Serial.println("Wakeup sha204a...");Serial.flush();
      SHA204response = sha204dev.resync(4, rx_buffer); //better than sha204dev.wakeup(rx_buffer);
      Serial.printf("Wakeup response = 0x%02X\n", SHA204response);Serial.flush();

// Get a Nonce, need to put 20 bytes.
// ********************************************************************************************************************* //
      Serial.print("Nonce: ");
      // Send SHA1 and get once
      SHA204response = sha204dev.nonce(tx_buffer, rx_buffer, 0, reqsha);

      if (SHA204response != 0){
         Serial.printf("Error 0x%02X\n", SHA204response);Serial.flush();
      }else{
        for (int i=1; i<sizeof(rx_buffer)-2; i++) { 
              Serial.printf("%02X", rx_buffer[i]);
        }
        Serial.println();Serial.flush();  
      }
      // copy rx_buffer from byte 1 to byte 33 to char nonce var
      memcpy(c_nonce,rx_buffer+1,32);

// Get HMAC
// ********************************************************************************************************************* //
      Serial.print("HMAC:  ");
      SHA204response = sha204dev.hmac(tx_buffer, rx_buffer, 0, 2);
    
      if (SHA204response != 0){
        Serial.printf("Error 0x%02X\n", SHA204response);Serial.flush();
      }else{
        for (int i=1; i<sizeof(rx_buffer)-2; i++) {
              Serial.printf("%02X", rx_buffer[i]);
        }
        Serial.println();Serial.flush();  
      }
      // copy rx_buffer from byte 1 to byte 33 to char hmac var
      memcpy(c_hmac,rx_buffer+1,32);

// Generate Autorizacion header:
//   - id 9 bytes (read from serial number)
//   - c_nonce 32 bytes ( sha204a device generated)
//   - base 24 bytes    ( const or compute from SN and mode)
//   - c_hmac 32 bytes  ( sha204a device generated)
// ********************************************************************************************************************* //
      char temp[100]; memset(temp,0,100);
        
      String authorization = "Authorization: 11PATHS-HMAC-256 ";

      // add base64 encode of SHA204A serial number to id
      base64_encode(temp,(char*)SHA204serialNumber,SHA204_SERIAL_SIZE);      
      authorization += "id=\"" + String(temp) + "\"";  memset(temp,0,100);

      // add base64 encode of nonce
      base64_encode(temp,(char*)c_nonce,32); 
      authorization += ", nonce=\"" + String(temp) + "\""; memset(temp,0,100);

      // add base64 encode of base
      base64_encode(temp,(char*)base,24); 
      authorization += ", base=\"" + String(temp) + "\""; memset(temp,0,100);

      // add base64 encode of hmac signature
      base64_encode(temp,(char*)c_hmac,32); 
      authorization += ", signature=\"" + String(temp) + "\"";

      Serial.print("Authorization Header: ");
      Serial.println(authorization);

// Send HTTP request
// ********************************************************************************************************************* //
      HTTPSclient.print(request);
      HTTPSclient.print("Host: ");
      HTTPSclient.print(remoteHost);HTTPSclient.print("\r\n");
      HTTPSclient.print("User-Agent: ArduinoESP/1.0\r\n");
      HTTPSclient.print(authorization);HTTPSclient.print("\r\n");
      HTTPSclient.print("Connection: close\r\n");
      HTTPSclient.print("\r\n");

// Get response headers
// ********************************************************************************************************************* //
     Serial.println("\nHeaders start>");Serial.flush();
     while (HTTPSclient.connected()) {
          String line = HTTPSclient.readStringUntil('\n');
          if (line=="\r"){
            break;
          }else{
            Serial.print(line);
          }
     }
     Serial.println("<Headers end.");

// Get response
// ********************************************************************************************************************* //
     Serial.println("\nResponse start>");Serial.flush();
     while (HTTPSclient.connected()) {
        String line = HTTPSclient.readStringUntil('\n');
        if (line!=""){
          Serial.println(line);
        }else{
          Serial.print("\n");
        }
     }
     Serial.println("<Response end.");
   }
   HTTPSclient.flush();
   HTTPSclient.stop();
   delay(1000);
   ticker.attach(1.0, tick);
 }
}
