#include <HardwareSerial.h>
#include <WiFi.h>

#define BUFFER_SIZE 100 
#define UART1_RX 18
#define UART1_TX 17

//Variáveis da comunicação serial
HardwareSerial UART_COM(1);
long ultimaConexaoSerial = 0;
int tempoMaximoNaoRetornoSerial = 2000;

int estadoEsp32 = 0;

//Variáveis da rede
//1 - wifi
int tipoRede;
char * nomeRede;
char * senhaRede;

//Variáveis da rede wifi master
int estadoWifiA = 0;
WiFiServer * server;
WiFiClient serverClient;
long ultimaConexaoWifiSlave;
char bufferMensagemParaSlave[BUFFER_SIZE];
char bufferMensagemDoSlave[BUFFER_SIZE];
bool novaMensagemParaSlave = false;
bool novaMensagemDoSlave = false;

//Variáveis da rede wifi slave
int estadoWifiB = 0;
WiFiClient * client;
long ultimaConexaoWifiMaster;
char bufferMensagemParaRaspSlave[BUFFER_SIZE];
char bufferMensagemDoRaspSlave[BUFFER_SIZE];
bool novaMensagemParaRaspSlave = false;
bool novaMensagemDoRaspSlave = false;

void(* resetFunc) (void) = 0;

void setup() 
{
  Serial.begin(115200);
  UART_COM.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

  ClearSerial();
}

void loop() 
{
  if (UART_COM.available() > 0) {
    char buffer[BUFFER_SIZE];
    int index = 0;
    
    while (UART_COM.available() > 0) {
      buffer[index++] = UART_COM.read();
    }
    buffer[index] = '\0';

    if (index < 3 || buffer[0] != 'C')
      return;
    
    ultimaConexaoSerial = millis();
    
    if (strcmp(buffer, "C00") == 0) {
      if (estadoEsp32 != 0)
        resetFunc();

      UART_COM.write("C01");
      estadoEsp32 = 1;

      Serial.println("Raspberry Conectada");
    }
    else if (estadoEsp32 == 0) {
      return;
    }
    else if (estadoEsp32 == 1) {
      Serial.print("Mensagem Recebida: ");
      Serial.println(buffer);

      if (strcmp(buffer, "C02") == 0)
        UART_COM.write("C02");
      else if (strcmp(buffer, "C03") == 0) {
        UART_COM.write("C03:OK");
        estadoEsp32 = 2;
      }
    }
    else if (estadoEsp32 >= 2 && estadoEsp32 <= 4) {
      Serial.print("Mensagem Recebida: ");
      Serial.println(buffer);

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
          break;
        case 3:
          nomeRede = strdup(dadosRede);
          break;
        case 4:
          senhaRede = strdup(dadosRede);
          break;
      }

      estadoEsp32++;

      if (estadoEsp32 == 5) {
        Serial.print("\nDados da rede: \nTipo: ");
        Serial.print(tipoRede);
        Serial.print("; Nome da Rede: ");
        Serial.print(nomeRede);
        Serial.print("; Senha da Rede: ");
        Serial.println(senhaRede);

        tempoMaximoNaoRetornoSerial = 10000;

        if (tipoRede == 1) {
          estadoEsp32 = 6;
          estadoWifiA = 1;
        }
        else if (tipoRede == 2) {
          estadoEsp32 = 106;
          estadoWifiB = 1;
        }
      }

      UART_COM.write("C03:OK");
    }
    else if (estadoEsp32 == 6) {
      Serial.print("Mensagem Recebida: ");
      Serial.println(buffer);

      if (strcmp(buffer, "C04") == 0)
        UART_COM.write("C04");
    }
    else if (estadoEsp32 == 7) {
      Serial.print("Mensagem Recebida: ");
      Serial.println(buffer);

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
          strcpy(bufferMensagemParaSlave, buffer);
          novaMensagemParaSlave = true;
        }

        if (novaMensagemDoSlave) {
          novaMensagemDoSlave = false;
          UART_COM.write(bufferMensagemDoSlave);
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
          strcpy(bufferMensagemDoRaspSlave, buffer);
          novaMensagemDoRaspSlave = true;
        }

        if (novaMensagemParaRaspSlave) {
          novaMensagemParaRaspSlave = false;
          UART_COM.write(bufferMensagemParaRaspSlave);
        }
        else
          UART_COM.write("C06");
      }
    }
  }

  if (estadoWifiA == 1) {
    server = new WiFiServer(80);

    WiFi.softAP(nomeRede, senhaRede);
    IPAddress IP = WiFi.softAPIP();

    Serial.print("AP IP address: ");
    Serial.println(IP);
    
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
      if (novaMensagemParaSlave) {
        novaMensagemParaSlave = false;
        serverClient.write(bufferMensagemParaSlave, BUFFER_SIZE);
      }
      else
        serverClient.write("C07", BUFFER_SIZE);

      ultimaConexaoWifiSlave = millis();
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
          strcpy(bufferMensagemDoSlave, buffer);
          novaMensagemDoSlave = true;
        }

        ultimaConexaoWifiSlave = millis();
        estadoWifiA = 3;
      }
    }
  }

  if (estadoWifiB == 1) {
    client = new WiFiClient();
    WiFi.begin(nomeRede, senhaRede);

    estadoWifiB = 2;
  }
  else if (estadoWifiB == 2) {
    if (WiFi.status() == WL_CONNECTED) {
      estadoWifiB = 3;

      Serial.print("Wifi conectado com IP: ");
      Serial.println(WiFi.localIP());
    }
  }
  else if (estadoWifiB == 3) {
    if (client->connect(WiFi.gatewayIP(), 80)) {
      Serial.println("Conectado com o servidor");
      ultimaConexaoWifiMaster = millis();

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
        novaMensagemParaRaspSlave = true;
        strcpy(bufferMensagemParaRaspSlave, buffer);
      }

      if (novaMensagemDoRaspSlave) {
        novaMensagemDoRaspSlave = false;
        client->write(bufferMensagemDoRaspSlave, BUFFER_SIZE);
      }
      else
        client->write("C07", BUFFER_SIZE);
      ultimaConexaoWifiMaster = millis();
    }
  }

  if (estadoEsp32 != 0 && millis() - ultimaConexaoSerial > tempoMaximoNaoRetornoSerial) {
    resetFunc();
  }

  if (estadoWifiA >= 4 && millis() - ultimaConexaoWifiSlave > tempoMaximoNaoRetornoSerial) {
    resetFunc();
  }

  if (estadoWifiB >= 4 && millis() - ultimaConexaoWifiMaster > tempoMaximoNaoRetornoSerial) {
    resetFunc();
  }
}

void ClearSerial() {
  while (UART_COM.available())
    UART_COM.read();
}