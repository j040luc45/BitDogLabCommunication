#include <WiFi.h>

#define BUFFER_SIZE 50 
#define TIME_To_RESET 3000

const char* ssid     = "ESP32-Master";
const char* password = "123456789";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

long ultimaConexao = 0;

// the loop function runs over and over again forever
void loop() {
  if (millis() - ultimaConexao > 5000) {
    resetFunc();
    return;
  }

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,

    Serial.println("New Client.");          // print a message out in the serial port

    while (client.connected()) {            // loop while the client's connected

      if (client.available()) {   

        ultimaConexao = millis();

        char buffer[BUFFER_SIZE];
        char caracter = client.read();

        for (int i = 0; i < BUFFER_SIZE && caracter != '\n'; i++) {
          buffer[i] = caracter;
          caracter = client.read();
        }

        Serial.println(buffer);
        client.write("Hello from ESP32 Master!", BUFFER_SIZE);
      }
    }

    client.flush();
  }
}









