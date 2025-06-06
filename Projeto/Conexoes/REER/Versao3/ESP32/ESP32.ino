#include <HardwareSerial.h>
#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>

#include "LoRaWANTool.h"

#define BUFFER_SIZE 100 

//ESP32-C3 //Usando Pinos SDA e SCL
//#define UART1_RX 6
//#define UART1_TX 7

//ESP32-C3
#define UART1_RX 20
#define UART1_TX 21

//ESP32-S3
//#define UART1_RX 18
//#define UART1_TX 17

//Comunicação LoRa
#define SS 2
#define RST 4
#define DIO0 3

//Variáveis da comunicação serial
HardwareSerial UART_COM(1);
long ultimaConexaoSerial = 0;
int tempoMaximoNaoRetornoSerial = 2000;

int estadoEsp32 = 0;

//Variáveis da rede
//Variáveis comuns de rede
char bufferMensagemParaESP32[BUFFER_SIZE];
char bufferMensagemParaRaspberry[BUFFER_SIZE];
bool novaMensagemParaESP32 = false;
bool novaMensagemParaRaspberry = false;
long ultimaConexaoWireless;

//1 - wifi
int tipoRede;
char * nomeRede;
char * senhaRede;
int tempoMaximoNaoRetornoWifi = 2000;

//Variáveis da rede wifi master
int estadoWifiA = 0;
WiFiServer * server;
WiFiClient serverClient;

//Variáveis da rede wifi slave
int estadoWifiB = 0;
WiFiClient * client;

//2 - LoRa
int estadoLoRaA = 0;
int estadoLoRaB = 0;
int tempoMaximoNaoRetornoLoRa = 10000;

//3 - LoRaWAN
int estadoLoRaWAN = 0;

void(* resetFunc) (void) = 0;

void reiniciar() {
  ClearSerial();
  UART_COM.end();
  delay(2000);

  resetFunc();
}

void setup() {
  Serial.begin(115200);
  UART_COM.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

  Serial.println("Inicializando");

  ClearSerial();
}

void loop() {
  if (UART_COM.available() > 0) {
    char buffer[BUFFER_SIZE];
    int index = 0;
    
    while (UART_COM.available() > 0) {
      buffer[index++] = UART_COM.read();
    }
    buffer[index] = '\0';

    Serial.print("Mensagem Recebida: ");
    Serial.println(buffer);

    if (index < 3 || buffer[0] != 'C')
      return;
    
    ultimaConexaoSerial = millis();
    
    if (strcmp(buffer, "C00") == 0) {
      if (estadoEsp32 != 0)
        reiniciar();

      UART_COM.write("C01");
      estadoEsp32 = 1;
    }
    else if (estadoEsp32 == 0) {
      return;
    }
    else if (estadoEsp32 == 1) {
      if (strcmp(buffer, "C02") == 0)
        UART_COM.write("C02");
      else if (strcmp(buffer, "C03") == 0) {
        UART_COM.write("C03:OK");
        estadoEsp32 = 2;
      }
    }
    else if (estadoEsp32 >= 2 && estadoEsp32 <= 4) {
      if (buffer[0] != 'C' || buffer[1] != '0' || buffer[2] != '3')
        return;

      char dadosRede[BUFFER_SIZE];
      int i;
      for (i = 4; buffer[i] != '\0'; i++) {
        dadosRede[i - 4] = buffer[i];
      }
      dadosRede[i - 4] = '\0';

      switch (estadoEsp32) {
        case 2: 
          if (strcmp(dadosRede, "wifi-master") == 0)
            tipoRede = 1;
          else if (strcmp(dadosRede, "wifi-slave") == 0)
            tipoRede = 2;
          else if (strcmp(dadosRede, "lora-master") == 0)
            tipoRede = 3;
          else if (strcmp(dadosRede, "lora-slave") == 0)
            tipoRede = 4;
          else if (strcmp(dadosRede, "lorawan-uplink") == 0)
            tipoRede = 5;
          break;
        case 3:
          nomeRede = strdup(dadosRede);
          break;
        case 4:
          senhaRede = strdup(dadosRede);
      }

      estadoEsp32++;

      if (estadoEsp32 == 5) {
        /*
        Serial.print("\nDados da rede: \nTipo: ");
        Serial.print(tipoRede);
        Serial.print("; Nome da Rede: ");
        Serial.print(nomeRede);
        Serial.print("; Senha da Rede: ");
        Serial.println(senhaRede);
        */

        tempoMaximoNaoRetornoSerial = 10000;

        if (tipoRede == 1) {
          estadoEsp32 = 6;
          estadoWifiA = 1;
        }
        else if (tipoRede == 2) {
          estadoEsp32 = 106;
          estadoWifiB = 1;
        }
        else if (tipoRede == 3) {
          estadoEsp32 = 206;
          estadoLoRaA = 1;
        }
        else if (tipoRede == 4) {
          estadoEsp32 = 306;
          estadoLoRaB = 1;
        }
        else if (tipoRede == 5) {
          setIntervaloLoRaWAN(atoi(nomeRede));
          tempoMaximoNaoRetornoSerial = atoi(senhaRede);
          estadoEsp32 = 406;
          estadoLoRaWAN = 1;
        }
      }

      UART_COM.write("C03:OK");
    }
    else if (estadoEsp32 == 6) {
      if (strcmp(buffer, "C04") == 0)
        UART_COM.write("C04");
    }
    else if (estadoEsp32 == 7) {
      if (strcmp(buffer, "C04") == 0){
        UART_COM.write("C04:OK");

        estadoEsp32 = 8;
        tempoMaximoNaoRetornoSerial = 2000;
      }
    }
    else if (estadoEsp32 == 8) {
      if (strcmp(buffer, "C05") == 0)
        UART_COM.write("C05");
    }
    else if (estadoEsp32 == 9) {
      if (strcmp(buffer, "C05") == 0) {
        UART_COM.write("C05:OK");
        estadoEsp32 = 10;
      }
    }
    else if (estadoEsp32 == 10) {
      if (buffer[0] == 'C' && buffer[1] == '0' && (buffer[2] == '6' || buffer[2] == '8')) {
        if (buffer[2] == '8') {
          strcpy(bufferMensagemParaESP32, buffer);
          novaMensagemParaESP32 = true;
        }

        if (novaMensagemParaRaspberry) {
          novaMensagemParaRaspberry = false;
          UART_COM.write(bufferMensagemParaRaspberry);
        }
        else
          UART_COM.write("C06");
      }
    }
    else if (estadoEsp32 == 106) {
      if (strcmp(buffer, "C05") == 0)
        UART_COM.write("C05");
    }
    else if (estadoEsp32 == 107) {
      if (strcmp(buffer, "C05") == 0) {
        UART_COM.write("C05:OK");
        
        estadoEsp32 = 108;
        tempoMaximoNaoRetornoSerial = 2000;
      }
    }
    else if (estadoEsp32 == 108) {
      if (buffer[0] == 'C' && buffer[1] == '0' && (buffer[2] == '6' || buffer[2] == '8')) {
        if (buffer[2] == '8') {
          strcpy(bufferMensagemParaESP32, buffer);
          novaMensagemParaESP32 = true;
        }

        if (novaMensagemParaRaspberry) {
          novaMensagemParaRaspberry = false;
          UART_COM.write(bufferMensagemParaRaspberry);
        }
        else
          UART_COM.write("C06");
      }
    }
    else if (estadoEsp32 == 206) {
      if (strcmp(buffer, "C04") == 0)
        UART_COM.write("C04");
    }
    else if (estadoEsp32 == 207) {
      if (strcmp(buffer, "C04") == 0){
        UART_COM.write("C04:OK");

        estadoEsp32 = 208;
        tempoMaximoNaoRetornoSerial = 2000;
      }
    }
    else if (estadoEsp32 == 208) {
      if (strcmp(buffer, "C05") == 0)
        UART_COM.write("C05");
    }
    else if (estadoEsp32 == 209) {
      if (strcmp(buffer, "C05") == 0) {
        UART_COM.write("C05:OK");
        estadoEsp32 = 210;

        tempoMaximoNaoRetornoSerial = atoi(senhaRede);
      }
    }
    else if (estadoEsp32 == 210) {
      if (buffer[0] == 'C' && buffer[1] == '0' && (buffer[2] == '6' || buffer[2] == '8')) {
        if (buffer[2] == '8') {
          strcpy(bufferMensagemParaESP32, buffer);
          novaMensagemParaESP32 = true;
        }

        if (novaMensagemParaRaspberry) {
          novaMensagemParaRaspberry = false;
          UART_COM.write(bufferMensagemParaRaspberry);
        }
        else
          UART_COM.write("C06");
      }
    }
    else if (estadoEsp32 == 306) {
      if (strcmp(buffer, "C05") == 0)
        UART_COM.write("C05");
    }
    else if (estadoEsp32 == 307) {
      if (strcmp(buffer, "C05") == 0) {
        UART_COM.write("C05:OK");
        
        estadoEsp32 = 308;
        tempoMaximoNaoRetornoSerial = atoi(senhaRede);
      }
    }
    else if (estadoEsp32 == 308) {
      if (buffer[0] == 'C' && buffer[1] == '0' && (buffer[2] == '6' || buffer[2] == '8')) {
        if (buffer[2] == '8') {
          strcpy(bufferMensagemParaESP32, buffer);
          novaMensagemParaESP32 = true;
        }

        if (novaMensagemParaRaspberry) {
          novaMensagemParaRaspberry = false;
          UART_COM.write(bufferMensagemParaRaspberry);
        }
        else
          UART_COM.write("C06");
      }
    }

    else if (estadoEsp32 == 406) {
      if (strcmp(buffer, "C05") == 0)
        UART_COM.write("C05");
    }
    else if (estadoEsp32 == 407) {
      if (strcmp(buffer, "C05") == 0)
        UART_COM.write("C05:OK");

      estadoEsp32 = 408;
    }
    else if (estadoEsp32 == 408) {
      if (buffer[0] == 'C' && buffer[1] == '0' && (buffer[2] == '6' || buffer[2] == '8')) {
        if (buffer[2] == '8') {
          setMensagem(buffer);
        }
        
        UART_COM.write("C06");
      }
    }
    else if (estadoEsp32 == 409) {
      reiniciar();
    }
  }

  if (estadoWifiA > 0) {
    if (estadoWifiA == 1) {
      server = new WiFiServer(80);

      WiFi.softAP(nomeRede, senhaRede);
      IPAddress IP = WiFi.softAPIP();

      //Serial.print("AP IP address: ");
      //Serial.println(IP);
      
      server->begin();
      estadoWifiA = 2;
      estadoEsp32 = 7;
    }
    else if (estadoWifiA == 2) {
      serverClient = server->available();

      if (serverClient) {
        estadoWifiA = 3;
        estadoEsp32 = 9;
      }
    }
    else if (estadoWifiA == 3) {
      if (serverClient.connected()) {
        if (novaMensagemParaESP32) {
          novaMensagemParaESP32 = false;
          serverClient.write(bufferMensagemParaESP32, BUFFER_SIZE);
        }
        else
          serverClient.write("C07", BUFFER_SIZE);

        ultimaConexaoWireless = millis();
        estadoWifiA = 4;
      }
    } 
    else if (estadoWifiA == 4) {
      if (serverClient.connected() && serverClient.available()) {
        char buffer[BUFFER_SIZE];
        char caracter = serverClient.read();

        for (int i = 0; i < BUFFER_SIZE && caracter != '\n'; i++) {
          buffer[i] = caracter;
          caracter = serverClient.read();
        }

        if (buffer[0] == 'C' && buffer[1] == '0') {
          if (buffer[2] == '8') {
            strcpy(bufferMensagemParaRaspberry, buffer);
            novaMensagemParaRaspberry = true;
          }

          ultimaConexaoWireless = millis();
          estadoWifiA = 3;
        }
      }
    }
  }

  if (estadoWifiB > 0) {

    if (estadoWifiB == 1) {
      client = new WiFiClient();
      WiFi.begin(nomeRede, senhaRede);

      estadoWifiB = 2;
    }
    else if (estadoWifiB == 2) {
      if (WiFi.status() == WL_CONNECTED) {
        estadoWifiB = 3;

        //Serial.print("Wifi conectado com IP: ");
        //Serial.println(WiFi.localIP());
      }
    }
    else if (estadoWifiB == 3) {
      if (client->connect(WiFi.gatewayIP(), 80)) {
        ultimaConexaoWireless = millis();

        estadoEsp32 = 107;
        estadoWifiB = 4;
      }
    }
    else if (estadoWifiB == 4) {
      if (client->available()) {
        char buffer[BUFFER_SIZE];
        char caracter = client->read();

        for (int i = 0; i < BUFFER_SIZE && caracter != '\n'; i++) {
          buffer[i] = caracter;
          caracter = client->read();
        }

        if (buffer[0] == 'C' && buffer[1] == '0' && buffer[2] == '8') {
          novaMensagemParaRaspberry = true;
          strcpy(bufferMensagemParaRaspberry, buffer);
        }

        if (novaMensagemParaESP32) {
          novaMensagemParaESP32 = false;
          client->write(bufferMensagemParaESP32, BUFFER_SIZE);
        }
        else
          client->write("C07", BUFFER_SIZE);

        ultimaConexaoWireless = millis();
      }
    }

  }

  if (estadoLoRaA > 0) {

    if (estadoLoRaA == 1){
      LoRa.setPins(SS, RST, DIO0);
      estadoLoRaA = 2;
    }
    else if (estadoLoRaA == 2) {
      if (LoRa.begin(915E6)) {
        long hexValue = strtol(nomeRede, NULL, 0);
        Serial.println(hexValue);
        LoRa.setSyncWord(hexValue);

        estadoLoRaA = 3;
        estadoEsp32 = 207;
      }
    }
    else if (estadoLoRaA == 3) {
      LoRa.beginPacket();
      LoRa.print("C07");
      LoRa.endPacket();

      Serial.println("Enviando tentativa de conexão");
      estadoLoRaA = 4;
      ultimaConexaoWireless = millis();
    }
    else if (estadoLoRaA == 4) {
      int packetSize = LoRa.parsePacket();

      if (packetSize) {
        Serial.println("Cliente Retornou");
        
        String LoRaData = String("");
        while (LoRa.available()) {
          LoRaData = LoRa.readString();
        }

        if (LoRaData == "C07"){
          estadoEsp32 = 209;
          estadoLoRaA = 5;
          ultimaConexaoWireless = millis();
        }
      }
      else if (millis() - ultimaConexaoWireless > tempoMaximoNaoRetornoLoRa)
        estadoLoRaA = 3;
    }
    else if (estadoLoRaA == 5) {
      LoRa.beginPacket();
      if (novaMensagemParaESP32) {
          novaMensagemParaESP32 = false;
          LoRa.print(bufferMensagemParaESP32);
        }
        else
          LoRa.print("C07");
      LoRa.endPacket();

      estadoLoRaA = 6;
      ultimaConexaoWireless = millis();
    }
    else if (estadoLoRaA == 6) {
      int packetSize = LoRa.parsePacket();

      if (packetSize) {
        String LoRaData = String("");
        while (LoRa.available()) {
          LoRaData = LoRa.readString();
        }

        if (LoRaData.indexOf("C08") >= 0) {
          const char * buffer = LoRaData.c_str();
          strcpy(bufferMensagemParaRaspberry, buffer);

          novaMensagemParaRaspberry = true;

          estadoLoRaA = 5;
          ultimaConexaoWireless = millis();
        }
        else if (LoRaData == "C07") {
          estadoLoRaA = 5;
          ultimaConexaoWireless = millis();
        }
      }
      else if (millis() - ultimaConexaoWireless > tempoMaximoNaoRetornoLoRa / 4)
        estadoLoRaA = 5;
    }

  }

  if (estadoLoRaB > 0) {

    if (estadoLoRaB == 1) {
      LoRa.setPins(SS, RST, DIO0);
      estadoLoRaB = 2;
    }
    else if (estadoLoRaB == 2) {
      if (LoRa.begin(915E6)) {
        long hexValue = strtol(nomeRede, NULL, 0);
        Serial.println(hexValue);
        LoRa.setSyncWord(hexValue);

        estadoLoRaB = 3;
      }
    }
    else if (estadoLoRaB == 3) {
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        // received a packet
        Serial.println("Pacote Recebido");

        // read packet
        String LoRaData;
        while (LoRa.available()) {
          LoRaData = LoRa.readString();
          Serial.println(LoRaData); 
        }

        if (LoRaData == "C07") {
          Serial.println("Tentativa de Conexão Recebida!");

          LoRa.beginPacket();
          LoRa.print("C07");
          LoRa.endPacket();
          
          ultimaConexaoWireless = millis();
          estadoEsp32 = 307;
          estadoLoRaB = 4;
        }
      }
    }
    else if (estadoLoRaB == 4) {
      int packetSize = LoRa.parsePacket();
      if (packetSize) {

        String LoRaData = String("");
        while (LoRa.available()) {
          LoRaData = LoRa.readString();
        }

        if (LoRaData == "C07" || LoRaData.indexOf("C08") >= 0) {
          if (LoRaData.indexOf("C08") >= 0) {
            novaMensagemParaRaspberry = true;

            const char * buffer = LoRaData.c_str();
            strcpy(bufferMensagemParaRaspberry, buffer);
          }

          LoRa.beginPacket();

          if (novaMensagemParaESP32) {
            novaMensagemParaESP32 = false;
            LoRa.print(bufferMensagemParaESP32);
          }
          else
            LoRa.print("C07");

          LoRa.endPacket();
          
          ultimaConexaoWireless = millis();
        }
      }
    }

  }

  if (estadoLoRaWAN == 1) {
    estadoEsp32 = executarEtapaLoRaWAN(estadoEsp32);
  }

  if (estadoEsp32 != 0 && millis() - ultimaConexaoSerial > tempoMaximoNaoRetornoSerial) {
    reiniciar();
  }

  if (estadoWifiA >= 4 && millis() - ultimaConexaoWireless > tempoMaximoNaoRetornoWifi) {
    reiniciar();
  }

  if (estadoWifiB >= 4 && millis() - ultimaConexaoWireless > tempoMaximoNaoRetornoWifi) {
    reiniciar();
  }
  
  if (estadoLoRaA >= 5 && millis() - ultimaConexaoWireless > tempoMaximoNaoRetornoLoRa) {
    reiniciar();
  }
  
  if (estadoLoRaB >= 4 && millis() - ultimaConexaoWireless > tempoMaximoNaoRetornoLoRa) {
    reiniciar();
  }
}

void ClearSerial() {
  while (UART_COM.available())
    UART_COM.read();
}