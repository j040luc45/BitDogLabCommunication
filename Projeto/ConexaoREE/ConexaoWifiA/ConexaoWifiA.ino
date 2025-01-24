#include <HardwareSerial.h>
#include <WiFi.h>

#define BUFFER_SIZE 100 
#define UART1_RX 18
#define UART1_TX 17

//Variáveis da comunicação serial
HardwareSerial UART_COM(1);
char buffer[BUFFER_SIZE];
long ultimaConexaoSerial = 0;
int tempoMaximoNaoRetornoSerial = 2000;

int estadoEsp32A = 0;

//Variáveis da rede
//1 - wifi
int tipoRede;
char * nomeRede;
char * senhaRede;

//Variáveis da rede wifi
int estadoWifiA = 0;
WiFiServer * server;

void(* resetFunc) (void) = 0;

void setup() 
{
  Serial.begin(115200);
  UART_COM.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

  ClearBuffer();
}

void loop() 
{
  if (UART_COM.available() > 0) {
    int index = 0;
    
    while (UART_COM.available() > 0) {
      buffer[index++] = UART_COM.read();
    }
    buffer[index] = '\0';

    if (index < 3 || buffer[0] != 'C')
      return;
    
    ultimaConexaoSerial = millis();
    
    if (strcmp(buffer, "C00") == 0) {
      UART_COM.write("C01");
      estadoEsp32A = 1;

      Serial.println("Raspberry Conectada");
    }
    else if (estadoEsp32A == 0)
      return;
    else if (estadoEsp32A == 1) {
      Serial.print("Mensagem Recebida: ");
      Serial.println(buffer);

      if (strcmp(buffer, "C02") == 0)
        UART_COM.write("C02");
      else if (strcmp(buffer, "C03") == 0) {
        UART_COM.write("C03:OK");
        estadoEsp32A = 2;
      }
    }
    else if (estadoEsp32A >= 2 && estadoEsp32A <= 4) {
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

      switch (estadoEsp32A) {
        case 2: 
          if (strcmp(dadosRede, "wifi") == 0)
            tipoRede = 1;
          break;
        case 3:
          nomeRede = strdup(dadosRede);
          break;
        case 4:
          senhaRede = strdup(dadosRede);
          break;
      }

      UART_COM.write("C03:OK");
      estadoEsp32A++;

      if (estadoEsp32A == 5) {

        Serial.print("\nDados da rede: \nTipo: ");
        Serial.print(tipoRede);
        Serial.print("; Nome da Rede: ");
        Serial.print(nomeRede);
        Serial.print("; Senha da Rede: ");
        Serial.println(senhaRede);

        tempoMaximoNaoRetornoSerial = 10000;

        if (tipoRede == 1)
          estadoWifiA = 1;
      }
    }
    else if (estadoEsp32A == 5) {
      Serial.print("Mensagem Recebida: ");
      Serial.println(buffer);

      if (strcmp(buffer, "C04") == 0)
        UART_COM.write("C04");
    }
    else if (estadoEsp32A == 6) {
      Serial.print("Mensagem Recebida: ");
      Serial.println(buffer);

      if (strcmp(buffer, "C04") != 0)
        return;

      UART_COM.write("C04:OK");

      estadoEsp32A = 7;
      tempoMaximoNaoRetornoSerial = 2000;
    }
    else if (estadoEsp32A == 7) {
      if (strcmp(buffer, "C05") == 0)
        UART_COM.write("C05");
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
    estadoEsp32A = 6;
  }
  else if (estadoWifiA == 2) {

  }

  if (estadoEsp32A != 0 && millis() - ultimaConexaoSerial > tempoMaximoNaoRetornoSerial) {
    resetFunc();
  }
}

void ClearBuffer() {
  while (UART_COM.available())
    UART_COM.read();
}