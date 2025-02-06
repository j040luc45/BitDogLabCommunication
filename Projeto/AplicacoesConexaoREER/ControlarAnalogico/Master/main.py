from machine import Pin, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import neopixel
import time
import ESP32Ferramentas

# Número de LEDs na sua matriz 5x5
# Inicializar a matriz de NeoPixels no GPIO7
NUM_LEDS = 25
np = neopixel.NeoPixel(Pin(7), NUM_LEDS)

# Definindo a matriz de LEDs
LEDMATRIXINDEX = [
    [24, 23, 22, 21, 20],
    [15, 16, 17, 18, 19],
    [14, 13, 12, 11, 10],
    [5, 6, 7, 8, 9],
    [4, 3, 2, 1, 0]
]

# definir cores para os LEDs
PALETADECORES = {"RED": (50, 0, 0), "BLUE": (0, 0, 50), "YELLOW": (30, 30, 0), "MAGENTA": (30, 0, 30), "CYAN": (0, 30, 30), "WHITE": (25, 25, 25), "BLACK": (0, 0, 0)}

# Inicializar ADC para os pinos VRx (GPIO26) e VRy (GPIO27)
adc_vrx = ADC(Pin(27))
adc_vry = ADC(Pin(26))

# apagar todos os LEDs
def clear_all(neoPixel):
    for i in range(len(neoPixel)):
        neoPixel[i] = PALETADECORES["BLACK"]
    neoPixel.write()

def mapearValor(value, in_min, in_max, out_min, out_max):
    return round((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

clear_all(np)

"""
while True:
    #Xmed = 32250; 	Ymed = 32888
    #Xmin = 192; 	Ymin = 192
    #Xmax = 65247; 	Ymax = 65247


    analogicoX = adc_vrx.read_u16()
    analogicoY = adc_vry.read_u16()

    analogicoX = mapearValor(analogicoX, 192, 65247, 0, 4)
    analogicoY = mapearValor(analogicoY, 192, 65247, 0, 4)

    #clear_all(np)
    for iRow, row in enumerate(LEDMATRIXINDEX):
        for iCell, cell in enumerate(row):
            if (iRow == 4 - analogicoY and iCell == analogicoX):
                np[cell] = PALETADECORES["RED"]
            else:
                np[cell] = PALETADECORES["BLACK"]
    
    
    np.write()
"""

esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":wifi-master", ":ESP32-Master", ":123456789"], True)
lastTimeAtualizacaoAnalogico = time.ticks_ms()

while True:
    if(esp32.executarEtapa()):
        if (time.ticks_ms() - lastTimeAtualizacaoAnalogico > 100):
            lastTimeAtualizacaoAnalogico = time.ticks_ms()
            analogicoX = adc_vrx.read_u16()
            analogicoY = adc_vry.read_u16()

            analogicoX = mapearValor(analogicoX, 192, 65247, 0, 4)
            analogicoY = mapearValor(analogicoY, 192, 65247, 0, 4)

            esp32.limparDadosParaEnviar()
            esp32.enviarDados("ax:{0};ay:{1}".format(analogicoX, analogicoY))

    esp32.executarIntervalo()

