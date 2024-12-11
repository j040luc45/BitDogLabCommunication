#include <HardwareSerial.h>

#define BUFFER_SIZE 50 
#define UART1_RX 18
#define UART1_TX 17

HardwareSerial UART_COM(1);
char buffer[BUFFER_SIZE];

void setup() 
{
  Serial.begin(115200);
  UART_COM.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);
}

void loop() 
{
  if (UART_COM.available() > 0) {
    int index = 0;
    
    while (UART_COM.available() > 0) {
      buffer[index++] = UART_COM.read();
    }
    buffer[index] = '\0';

    Serial.print("Mensagem Recebida: ");
    Serial.println(buffer);
    
    UART_COM.write("Mensagem do Arduino");
  }
}