#include <HardwareSerial.h>

#define UART1_RX 20
#define UART1_TX 21

HardwareSerial UART_COM(1);

void setup() {
  Serial.begin(115200);    // Conecta ao computador via USB

  UART_COM.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);
}

void loop() {
  if (Serial.available()) {
    UART_COM.write(Serial.read());  // Passa dados do PC para a placa com defeito
  }
  if (UART_COM.available()) {
    Serial.write(UART_COM.read());  // Passa dados da placa com defeito para o PC
  }
}