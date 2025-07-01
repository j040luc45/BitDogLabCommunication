from machine import Pin, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import time
import ESP32Ferramentas
import data_for_lorawan

esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":lorawan-uplink", ":10", ":60000"], True)

while True:
    if(esp32.executarEtapa()):
        print(data_for_lorawan.data_lora())
        esp32.enviarDados(str(data_for_lorawan.data_lora()))