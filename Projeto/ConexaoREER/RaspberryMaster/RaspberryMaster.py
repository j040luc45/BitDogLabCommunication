from machine import Pin, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import time
import ESP32Ferramentas

esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":wifi-master", ":ESP32-Master", ":123456789"], True)

while True:
    if(esp32.executarEtapa()):
        esp32.enviarDados("Oxi estou aqui")

    esp32.executarIntervalo()