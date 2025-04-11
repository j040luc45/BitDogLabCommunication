from machine import Pin, SPI
import time

# Configuração do SPI (Pico como Slave)
spi = SPI(0, baudrate=500000, polarity=0, phase=0, firstbit=SPI.MSB,
          sck=Pin(18), mosi=Pin(19), miso=Pin(16))
cs = Pin(17, Pin.IN)  # Chip Select (deve estar no mesmo pino do Master)

buffer = bytearray(1)  # Buffer de 1 byte para troca de dados

while True:
    while (cs.value() != 0):
        time.sleep(0.001)
    
    while (cs.value() == 0):
        spi.readinto(buffer)  # Lê o dado enviado pelo Master
        print("Recebido do ESP32:", buffer[0])

        response = bytes([buffer[0] + 1])  # Exemplo: Retorna o valor recebido +1
        spi.write(response)  # Envia resposta ao Master