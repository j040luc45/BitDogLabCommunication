from machine import Pin, UART, SoftI2C
from ssd1306 import SSD1306_I2C # type: ignore
import ESP32Ferramentas # type: ignore
import time

esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":lora-slave", ":0xA5", ":20000"], True)

while True:
    if(esp32.executarEtapa()):
        esp32.enviarDados("Oxi estou aqui")

        novaMensagem = esp32.recuperadaDadosRecebido()
        if (novaMensagem is not None):
            print(novaMensagem)