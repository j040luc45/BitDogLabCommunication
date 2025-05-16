from machine import Pin, SPI
import time

# Configurar pinos SPI
spi = SPI(0, baudrate=5000000, polarity=0, phase=0,
          sck=Pin(14), mosi=Pin(15), miso=Pin(8))

cs = Pin(17, Pin.OUT)
cs.value(1)  # desativa chip select

def read_register(addr):
    cs.value(0)
    spi.write(bytearray([addr & 0x7F]))  # MSB 0 = leitura
    result = spi.read(1)
    cs.value(1)
    return result[0]

while True:
    version = read_register(0x42)  # REG_VERSION
    print("REG_VERSION = 0x{:02X}".format(version))
    if version == 0x12:
        print("✅ RFM95 / SX1276 detectado!")
    elif version == 0x00 or version == 0xFF:
        print("❌ Sem resposta, verifique conexões.")
    else:
        print("⚠️ Valor inesperado. Algo pode estar errado.")
    
    time.sleep(2)