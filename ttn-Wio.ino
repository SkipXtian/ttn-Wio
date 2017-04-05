//FOR THIS TO COMPILE YOU MUST DISABLE DEBUGGING IN CONFIG.H OF Arduino-lmic

/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

// https://github.com/matthijskooijman/arduino-lmic

//#define DEBUG
#ifdef DEBUG
 #define DEBUG_PRINTLN(x)  Serial.println(x)
 #define DEBUG_PRINT(x)  Serial.print(x)
#else
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINT(x)
#endif

#include <lmic.h>
#include <hal/hal.h>
#include <Wire.h>
#include "secrets.h" // moved secret keys referenced below to here

#define BME280_ADDRESS 0x76
#define RAIN_GUAGE_PIN 7

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 300; //5mins


void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static osjob_t sendjob;

unsigned long blinkDelay = 1000;
boolean ledState = HIGH;
const byte LED_PIN = 13;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {A0, A5, A4},
};

void onEvent (ev_t ev) {
   DEBUG_PRINT(os_getTime());
    DEBUG_PRINT(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            DEBUG_PRINTLN(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            DEBUG_PRINTLN(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            DEBUG_PRINTLN(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            DEBUG_PRINTLN(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            DEBUG_PRINTLN(F("EV_JOINING"));
            break;
        case EV_JOINED:
            DEBUG_PRINTLN(F("EV_JOINED"));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            DEBUG_PRINTLN(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            DEBUG_PRINTLN(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            DEBUG_PRINTLN(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:
            DEBUG_PRINTLN(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              DEBUG_PRINTLN(F("Received ack"));
            if (LMIC.dataLen) {
              DEBUG_PRINTLN(F("Received "));
              DEBUG_PRINTLN(LMIC.dataLen);
              DEBUG_PRINTLN(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            DEBUG_PRINTLN(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            DEBUG_PRINTLN(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            DEBUG_PRINTLN(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            DEBUG_PRINTLN(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            DEBUG_PRINTLN(F("EV_LINK_ALIVE"));
            break;
         default:
            DEBUG_PRINTLN(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        DEBUG_PRINTLN(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
//**********************************************************************************************************************
        int32_t temperature; 
        uint32_t pressure, humidity;
        getBMEData(&temperature, &pressure, &humidity);
    
        char seperator[] = ",";
        char tempBuf[6];
        ltoa(temperature, tempBuf, 10);
        char preBuf[8];
        ultoa(pressure, preBuf, 10);
        char humBuf[6];
        ultoa(humidity, humBuf, 10);
        char rainBuf[8];
        ultoa(calcUnitRain(), rainBuf, 10);
        char mydata [25] = {0};
        strcat(mydata, tempBuf);
        strcat(mydata, seperator);
        strcat(mydata, preBuf);
        strcat(mydata, seperator);
        strcat(mydata, humBuf);
        strcat(mydata, seperator);
        strcat(mydata, rainBuf);
        DEBUG_PRINTLN(mydata); 
        byte i = 0;
        for(i = 0; i < sizeof(mydata); i++){
          if(mydata[i] == '\0') {
            break;
          }
        }
//**********************************************************************************************************************        
        LMIC_setTxData2(1, (unsigned char *)mydata, i, 0);  //sizeof(mydata) (unsigned char *)
        DEBUG_PRINTLN(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

//****************************RAIN*****************************

#define RAIN_FACTOR 28 //0.2794  

volatile uint32_t rainTipper=0;
volatile uint32_t rainLastTip=0;
 
uint32_t calcUnitRain(){   
  uint32_t unitRain = rainTipper * RAIN_FACTOR;
  
  rainTipper=0; 
  
  DEBUG_PRINT(F("Rain Fall :"));
  DEBUG_PRINT(unitRain);
  DEBUG_PRINTLN(F("mm"));
  return unitRain;
}

void rainGageISR(){
  if(millis()-rainLastTip>400) {  //Stops debounce of the reed switch
    rainTipper += 1;
    rainLastTip=millis();
  }
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);
#endif
  DEBUG_PRINTLN(F("Starting WIO"));
//  pinMode(LED_PIN, OUTPUT);
//  digitalWrite(LED_PIN, HIGH);
  pinMode(RAIN_GUAGE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RAIN_GUAGE_PIN), rainGageISR, FALLING);
  Wire.begin();
  bmeInit();

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);
}

void loop() {
    os_runloop_once();
    blinkLED();
}

void blinkLED(){
  static unsigned long transistionTime = millis();
  if(millis() > transistionTime){
    ledState = !ledState;
    digitalWrite(13, ledState);
    transistionTime += blinkDelay;
  }
}
