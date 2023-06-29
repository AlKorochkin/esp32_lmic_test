#include <WiFi.h>
#include "esp_wifi.h"
#include <base64.h>
#include "driver/adc.h"

// All specific changes needed for ESP8266 need be made in hal.cpp if possible
// Include ESP environment definitions in lmic.h (lmic/limic.h) if needed
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include "esp_secrets.h"

//---------------------------------------------------------
// APPLICATION CALLBACKS
//---------------------------------------------------------

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}

// Pin mapping for RFM95

const lmic_pinmap lmic_pins = {
    .nss = 5,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = { /*dio0*/ 34, /*dio1*/ 35, /*dio2*/ LMIC_UNUSED_PIN }
    };

void onEvent (ev_t ev) {
    //debug_event(ev);
    switch(ev) {
      // scheduled data sent (optionally data received)
      // note: this includes the receive window!
      case EV_TXCOMPLETE:
          // use this event to keep track of actual transmissions
          Serial.print("Event EV_TXCOMPLETE, time: ");
          Serial.println(millis() / 1000);
          if(LMIC.dataLen) { // data received in rx slot after tx
              //debug_buf(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
              Serial.println("Data Received!");
          }
          break;
       default:
          break;
    }
}

#define SEND_MESSAGE 2.5*60*1000 // 3*60*1000
#define SENSOR_WARMUP 10000 //10000
#define SENSOR_READ 60*1000 // 70*1000

static osjob_t sendjob;


void sendData () {
    if (LMIC.opmode & (1 << 7)) {
      Serial.println("OP_TXRXPEND, not sending");
    } else {
      Serial.print("ok, ready to send: ");
      Serial.println();

      byte bytsend[10];                   // !!!! MAx 10 Bytes to send !!!!
      int idx = 0;
      
      int temp = 0;
      int hum = 0;

      bytsend[0] = 1;
      bytsend[1] = 2;
      bytsend[2] = 3;
      bytsend[3] = 4;
      bytsend[4] = 5;
      bytsend[5] = 6;
      bytsend[6] = 7;
      bytsend[7] = 8;
      bytsend[8] = 9;
      bytsend[9] = 10;

      uint8_t mydata[64];
      memcpy((char *)mydata, (char *)bytsend, 10);
      int k;
      for(k = 0; k < 10; k++) {
        Serial.print(mydata[k],HEX);
        Serial.print(" ");
      }

      Serial.println();

      // Prepare upstream data transmission at the next possible time.
      // LMIC_setTxData2(1, mydata, strlen((char *)mydata), 0);
      LMIC_setTxData2(1, mydata, 10, 0);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("");
    Serial.print("ChipID: ");
    Serial.print("DeviceID: ");
    Serial.println(DEVICEID);

    WiFi.mode(WIFI_MODE_NULL);
    esp_wifi_stop(); //you must do esp_wifi_start() the next time you'll need wifi or esp32 will crash
    adc_power_off();

    Serial.println("Init LMIC");
    // LMIC init
    os_init();
    Serial.println("Init LMIC Done");

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    Serial.println("LMIC SetSession");
    LMIC_setSession (0x1, DEVADDR, (uint8_t*)DEVKEY, (uint8_t*)ARTKEY);
    Serial.println("LMIC setAdrMod");
    // Disable data rate adaptation
    LMIC_setAdrMode(0);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);
    // Disable beacon tracking
    LMIC_disableTracking ();
    // Stop listening for downstream data (periodical reception)
    LMIC_stopPingable();
    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);
    
    Serial.println("Send Data");
    sendData();

}

void loop() {

}
