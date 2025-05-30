#include <HardwareSerial.h>

#define BUFFER_SIZE 50 
#define UART1_RX 20
#define UART1_TX 21

HardwareSerial UART_COM(1);
char buffer[BUFFER_SIZE];

void ClearBuffer();

void setup() 
{
  Serial.begin(115200);
  Serial.println("Inicializando...");
  UART_COM.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

  ClearBuffer();
}

void loop() 
{
  Serial.println("Oxi...");
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

void ClearBuffer() {

  while (UART_COM.available())
    UART_COM.read();

}