from machine import Pin, SoftI2C, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import time
import random

DIMENSAOXOLED = 128
DIMENSAOYOLED = 64

TAMANHOBARRAJOGADOR = 20
DISTANCIAJOGADORPAREDE = 3

TAMANHOBOLA = 2
VELOCIDADEBOLA = 3

# Inicializar ADC para os pinos VRx (GPIO26) e VRy (GPIO27)
adc_vrx = ADC(Pin(27))
adc_vry = ADC(Pin(26))

def mapearValor(value, in_min, in_max, out_min, out_max):
    return round((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

def leituraAnalogico():
    analogicoX = adc_vrx.read_u16()
    analogicoY = adc_vry.read_u16()

    analogicoX = mapearValor(analogicoX, 192, 65247, 0, 2)
    analogicoY = mapearValor(analogicoY, 192, 65247, 0, 2)

    return (analogicoX, analogicoY)

#OLED é 128x64
i2c = SoftI2C(scl=Pin(15), sda=Pin(14))
oled = SSD1306_I2C(DIMENSAOXOLED, DIMENSAOYOLED, i2c)

# Função gerada pelo ChatGPT
def drawFilledCircle(oled, x0, y0, radius, color=1):
    for y in range(-radius, radius + 1):
        for x in range(-radius, radius + 1):
            if x*x + y*y <= radius*radius:
                oled.pixel(x0 + x, y0 + y, color)

#limparOLED()
#oled.pixel(0, 0, 1)
#oled.invert(True)
#oled.fill_rect(10, 10, 30, 30, 1)
#drawFilledCircle(oled, 64, 32, 3)

def atualizarFrame(posYJ1, posYJ2, posXBola, posYBola):
    oled.fill(0)
    oled.fill_rect(DISTANCIAJOGADORPAREDE, posYJ1 - TAMANHOBARRAJOGADOR // 2, 2, TAMANHOBARRAJOGADOR, 1)
    drawFilledCircle(oled, posXBola, posYBola, TAMANHOBOLA) 
    oled.show()

posYJ1 = DIMENSAOYOLED // 2
posicaoBola = {"x": 64, "y": random.randint(5, DIMENSAOYOLED - 5)}
direcaoBola = {"x": 1, "y": 1}

while (True):
    _, valorYAnalogico = leituraAnalogico()
    posYJ1 -= (valorYAnalogico - 1) * 2
    posYJ1 = max(TAMANHOBARRAJOGADOR//2, min(DIMENSAOYOLED - TAMANHOBARRAJOGADOR//2, posYJ1))

    if (posicaoBola["x"] >= DIMENSAOXOLED - TAMANHOBOLA//2 and direcaoBola["x"] == 1):
        direcaoBola["x"] = -1
    elif (posicaoBola["x"] <= 0 + TAMANHOBOLA//2 and direcaoBola["x"] == -1):
        direcaoBola["x"] = 1
    
    if (posicaoBola["y"] >= DIMENSAOYOLED - TAMANHOBOLA//2 and direcaoBola["y"] == 1):
        direcaoBola["y"] = -1
    elif (posicaoBola["y"] <= 0 + TAMANHOBOLA//2 and direcaoBola["y"] == -1):
        direcaoBola["y"] = 1

    posicaoBola["x"] += direcaoBola["x"] * VELOCIDADEBOLA
    posicaoBola["y"] += direcaoBola["y"] * VELOCIDADEBOLA

    atualizarFrame(posYJ1, 0, posicaoBola["x"], posicaoBola["y"])

    time.sleep(1/30)

atualizarFrame(10, 0, 40, 40)
oled.show()