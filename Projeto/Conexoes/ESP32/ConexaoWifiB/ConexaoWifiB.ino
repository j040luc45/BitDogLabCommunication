#include <WiFi.h>

enum Protocol{
    PIN, //Pino que se deseja alterar o estado
    VALUE, //Estado para qual o pino deve ir (HIGH = 1 ou LOW = 0)
    BUFFER_SIZE //O tamanho do nosso protocolo. IMPORTANTE: deixar sempre como último do enum
};

#define BUFFER_SIZE 50 
#define TIME_OUT 100 

const char* ssid     = "ESP32-Master";
const char* password = "123456789";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("...");
  }

  Serial.print("WiFi connected with IP:");
  Serial.println(WiFi.localIP());
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

int tentativasConexao = 0;

void loop() {
  
  WiFiClient client;

  if (WiFi.status() != WL_CONNECTED || tentativasConexao == 5) {
    resetFunc();
    return;
  }

  if(!client.connect(WiFi.gatewayIP(), 80)){
    Serial.println("Connection to host failed");
    tentativasConexao++;
    delay(1000);
    return;
  }

  tentativasConexao = 0;

  while (true) {
    client.write("Hello from ESP32 Slave!", BUFFER_SIZE);

    bool semResposta = false;
    long ultimaComunicacao = millis();
    while (!client.available()) {
      if (millis() - ultimaComunicacao > TIME_OUT) {
        semResposta = true;
        break;
      }
    }

    if (semResposta)
      break;
    
    char buffer[BUFFER_SIZE];
    char caracter = client.read();

    for (int i = 0; i < BUFFER_SIZE && caracter != '\n'; i++) {
      buffer[i] = caracter;
      caracter = client.read();
    }

    Serial.println(buffer);

    delay(100);
  }

  client.flush();
  client.stop();

}










