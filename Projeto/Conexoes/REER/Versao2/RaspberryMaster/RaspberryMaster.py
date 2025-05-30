from machine import Pin, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import time
import ESP32Ferramentas

esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":lora-master", ":0xA5", ":0xA5"], True)

while True:
    if(esp32.executarEtapa()):
        esp32.enviarDados("Oxi estou aqui")

        novaMensagem = esp32.recuperadaDadosRecebido()
        if (novaMensagem is not None):
            print(novaMensagem)