#include <HardwareSerial.h>

#define BUFFER_SIZE 50 
#define UART1_RX 20
#define UART1_TX 21

HardwareSerial UART_COM(0);
char buffer[BUFFER_SIZE];

void setup() 
{
  UART_COM.begin(115200, SERIAL_8N1, RX, TX);
}

void loop() 
{
  UART_COM.println("Mensagem do Arduino");
  delay(1000);
}