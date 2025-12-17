
#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>

#define BUFFER_SIZE_LORA 200

static const u1_t PROGMEM APPEUI[8]={ 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

static const u1_t PROGMEM DEVEUI[8]={ 0x26, 0x43, 0x07, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

static const u1_t PROGMEM APPKEY[16] = { 0x56, 0x56, 0xF1, 0xB9, 0xD1, 0x32, 0x7F, 0x66, 0xA0, 0xF0, 0x67, 0xCE, 0xF9, 0x16, 0xD8, 0x77 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static osjob_t sendjob;

static int intervalorComunicacaoLoRaWAN = 60;
static uint8_t dadosLoRaWAN[BUFFER_SIZE_LORA];
static int maquinaLoRaWAN = 0;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 2,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 7, 6},
    .rxtx_rx_active = 0,
    .rssi_cal = 10,
    .spi_freq = 8000000,
    // Advanced configurations are passed to the pinmap via pConfig
    //.pConfig = &myConfig, //Talvez não precise
};

void printHex2(unsigned v) {
  v &= 0xff;
  if (v < 16)
      Serial.print('0');
  Serial.print(v, HEX);
}

void setIntervaloLoRaWAN(int intervalo) {
    intervalorComunicacaoLoRaWAN = intervalo;
}

void setMensagem(char * mensagem) {
  int i;
  for (i = 0; mensagem[i] != '\0' && i < BUFFER_SIZE_LORA; i++) {
    dadosLoRaWAN[i] = (uint8_t) mensagem[i];
  }
  dadosLoRaWAN[i] = (uint8_t) '\0';
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, dadosLoRaWAN, sizeof(dadosLoRaWAN)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            maquinaLoRaWAN = 4;
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            maquinaLoRaWAN = 2;
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	    // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            Serial.print("SNR: ");
            Serial.println(LMIC.snr);
            Serial.print("RSSI: ");
            Serial.println(LMIC.rssi);
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(intervalorComunicacaoLoRaWAN), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

int executarEtapaLoRaWAN(int estadoAtualEsp32) {

  if (maquinaLoRaWAN == 0) {
    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);

    maquinaLoRaWAN = 1;
  }
  else if (maquinaLoRaWAN != 0) {
    os_runloop_once();

    if (maquinaLoRaWAN == 2) {
        estadoAtualEsp32 = 407;
        maquinaLoRaWAN = 3;
    }
    else if (maquinaLoRaWAN == 4) {
        estadoAtualEsp32 = 409;
    }
  }

  return estadoAtualEsp32;
}

