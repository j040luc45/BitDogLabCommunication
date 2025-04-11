from machine import Pin, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import neopixel
import time
import ESP32Ferramentas # type: ignore

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
PALETADECORES = {"RED": (50, 0, 0), "BLUE": (0, 0, 50), "YELLOW": (30, 30, 0), "MAGENTA": (30, 0, 30),
                 "CYAN": (0, 30, 30), "WHITE": (25, 25, 25), "BLACK": (0, 0, 0)}

esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":wifi-slave", ":ESP32-Master", ":123456789"], True)

# apagar todos os LEDs
def clear_all(neoPixel):
    for i in range(len(neoPixel)):
        neoPixel[i] = PALETADECORES["BLACK"]
    neoPixel.write()

clear_all(np)

"""
while True:

    #clear_all(np)
    for iRow, row in enumerate(LEDMATRIXINDEX):
        for iCell, cell in enumerate(row):
            if (iRow == 4 - analogicoY and iCell == analogicoX):
                np[cell] = PALETADECORES["RED"]
            else:
                np[cell] = PALETADECORES["BLACK"]
    
    
    np.write()
"""

while True:
    if(esp32.executarEtapa()):
        dadoRecebido = esp32.RecuperadaDadoRecebido()

        if (dadoRecebido != "NaN"):
            valoresRecebidos = dadoRecebido.split(";")
            analogicoX = 2
            analogicoY = 2
            print("Valor Recebido: {0}".format(dadoRecebido))

            for valor in valoresRecebidos:
                if (valor[0:2] == "ax"):
                    analogicoX = int(valor[3:])
                elif (valor[0:2] == "ay"):
                    analogicoY = int(valor[3:])

            for iRow, row in enumerate(LEDMATRIXINDEX):
                for iCell, cell in enumerate(row):
                    if (iRow == 4 - analogicoY and iCell == analogicoX):
                        np[cell] = PALETADECORES["RED"]
                    else:
                        np[cell] = PALETADECORES["BLACK"]
            
            
            np.write()            

    esp32.executarIntervalo()