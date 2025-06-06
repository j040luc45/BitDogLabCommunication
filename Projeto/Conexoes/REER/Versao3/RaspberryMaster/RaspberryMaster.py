from machine import Pin, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import time
import ESP32Ferramentas

esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":lorawan-uplink", ":120", ":10000"], True)

while True:
    if(esp32.executarEtapa()):
        esp32.enviarDados("L:-22.5;l:-47.07;T:25.76;H:0;V:3.1;C:2.5")