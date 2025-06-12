from machine import Pin, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import time
import ESP32Ferramentas

esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":lorawan-uplink", ":120", ":10000", 
                                                   "0x01, 0x00, 0x00, 0x00, 0x7E, 0xD5, 0xB3, 0x70",
                                                   "0x4B, 0x14, 0x07, 0xD0, 0x7E, 0xD5, 0xB3, 0x70",
                                                   "0x43, 0x01, 0x91, 0x80, 0x1F, 0xDF, 0x2F, 0x74, 0xDB, 0x79, 0x83, 0xC1, 0xCF, 0x90, 0x88, 0x91"], 
                                                   True)
# Não Funcionando ainda APP e DEV por mensagem
while True:
    if(esp32.executarEtapa()):
        esp32.enviarDados("L:-22.5;l:-47.07;T:25.76;H:0;V:3.1;C:2.5")