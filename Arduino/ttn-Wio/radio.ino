void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {A0, A5, A4},
};

void onEvent (ev_t ev) 
{
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
            digitalWrite(LED_PIN, HIGH);
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
            digitalWrite(LED_PIN, LOW);
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

void do_send(osjob_t* j)
{
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        DEBUG_PRINTLN(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
//**********************************************************************************************************************  
        buildDataPacket();
        DEBUG_PRINTLN(mydata); 
        byte i = 0;
        for(i = 0; i < PACKET_SIZE; i++){
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


