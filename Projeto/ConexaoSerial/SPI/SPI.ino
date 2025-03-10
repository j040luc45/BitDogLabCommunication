#include <SPI.h>

#define CS_PIN 10  // Pino Chip Select

void setup() {
    Serial.begin(115200);
    SPI.begin();
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
}

void loop() {
    uint8_t dataToSend = 42;  // Exemplo de dado a ser enviado
    uint8_t receivedData = 0;

    digitalWrite(CS_PIN, LOW); // Seleciona o Slave
    delay(10);
    receivedData = SPI.transfer(dataToSend); // Envia e recebe ao mesmo tempo
    digitalWrite(CS_PIN, HIGH); // Libera o Slave

    Serial.print("Recebido do Pico: ");
    Serial.println(receivedData);

    delay(1000); // Aguarda antes de enviar outro dado
}