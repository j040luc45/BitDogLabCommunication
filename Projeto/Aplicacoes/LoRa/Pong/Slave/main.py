from machine import Pin, SoftI2C, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import time
import random
import ESP32Ferramentas # type: ignore

DIMENSAOXOLED = 128
DIMENSAOYOLED = 64

TAMANHOBARRAJOGADOR = 20
DISTANCIAJOGADORPAREDE = 3

TAMANHOBOLA = 2
VELOCIDADEBOLA = 2

# Inicializar ADC para os pinos VRx (GPIO26) e VRy (GPIO27)
adc_vrx = ADC(Pin(27))
adc_vry = ADC(Pin(26))

# Inicializar OLED
i2c = SoftI2C(scl=Pin(15), sda=Pin(14))
oled = SSD1306_I2C(DIMENSAOXOLED, DIMENSAOYOLED, i2c)
oled.fill(0)
tempoUltimaAtualziacao = time.ticks_ms()

# Inicializar Comunicação ESP32
esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":lora-slave", ":0xF1", ":0xF1"], True)
novaMessagemEsp = ""

# Configurações Iniciais
posYJ1 = DIMENSAOYOLED // 2
posYJ2 = DIMENSAOYOLED // 2
posicaoBola = [64, random.randint(5, DIMENSAOYOLED - 5)]
maquinaDeEstados = 0
estadoJogoMaster = 0
placarJogo = [0, 0]

def mapearValor(value, in_min, in_max, out_min, out_max):
    return round((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

def leituraAnalogico():
    analogicoX = adc_vrx.read_u16()
    analogicoY = adc_vry.read_u16()

    analogicoX = mapearValor(analogicoX, 192, 65247, 0, 2)
    analogicoY = mapearValor(analogicoY, 192, 65247, 0, 2)

    return (analogicoX, analogicoY)

# Função gerada pelo ChatGPT
def drawFilledCircle(oled, x0, y0, radius, color=1):
    for y in range(-radius, radius + 1):
        for x in range(-radius, radius + 1):
            if x*x + y*y <= radius*radius:
                oled.pixel(x0 + x, y0 + y, color)

def atualizarFrame(posYJ1, posYJ2, posXBola, posYBola):
    oled.fill(0)
    oled.fill_rect(DISTANCIAJOGADORPAREDE, posYJ1 - TAMANHOBARRAJOGADOR // 2, 2, TAMANHOBARRAJOGADOR, 1)
    oled.fill_rect(DIMENSAOXOLED - DISTANCIAJOGADORPAREDE - 2, posYJ2 - TAMANHOBARRAJOGADOR // 2, 2, TAMANHOBARRAJOGADOR, 1)
    drawFilledCircle(oled, posXBola, posYBola, TAMANHOBOLA) 
    oled.show()

while True:
    resultadoEtapa = esp32.executarEtapa()
    if (resultadoEtapa):
        if (maquinaDeEstados == 0):
            maquinaDeEstados = 1

        novaMensagem = esp32.recuperadaDadosRecebido()
        if (novaMensagem is not None):
            novaMessagemEsp = novaMensagem
    elif (resultadoEtapa == False):
        maquinaDeEstados = 0

    if (novaMessagemEsp != ""):
        posicaoElementosGraficos = novaMessagemEsp.split(";")
        estadoJogoMaster = int((posicaoElementosGraficos[0])[3:])

        if (estadoJogoMaster == 3 or estadoJogoMaster == 5):
            placarJogo[0] = int((posicaoElementosGraficos[1])[3:])
            placarJogo[1] = int((posicaoElementosGraficos[2])[3:])

        elif (estadoJogoMaster == 10):
            posicaoBola = (posicaoElementosGraficos[1])[2:]
            posicaoBola = (posicaoBola).split(",")
            posYJ1 = (posicaoElementosGraficos[2])[3:]
            novaMessagemEsp = ""

    if (time.ticks_ms() - tempoUltimaAtualziacao > 1000 // 30):
        tempoUltimaAtualziacao = time.ticks_ms()

        if (maquinaDeEstados == 0 or estadoJogoMaster == 0):
            continue

        if (estadoJogoMaster == 1):
            oled.fill(0)
            oled.text("Comecando Jogo", DIMENSAOXOLED // 2 - 55, DIMENSAOYOLED // 2 - 10)
            oled.text("0 - 0", DIMENSAOXOLED // 2 - 20, DIMENSAOYOLED // 2 + 2)
            oled.show()

        elif (estadoJogoMaster == 3 or estadoJogoMaster == 5):
            oled.fill(0)
            oled.text("Ponto Jogador 1", DIMENSAOXOLED // 2 - 60, DIMENSAOYOLED // 2 - 10)
            oled.text("{0} - {1}".format(placarJogo[0], placarJogo[1]), DIMENSAOXOLED // 2 - 20, DIMENSAOYOLED // 2 + 2)
            oled.show()

        elif (estadoJogoMaster == 20):
            oled.fill(0)
            oled.text("Vencedor", DIMENSAOXOLED // 2 - 30, DIMENSAOYOLED // 2 - 10)
            oled.text("Jogador 1", DIMENSAOXOLED // 2 - 33, DIMENSAOYOLED // 2 + 2)
            oled.text("Recomecando...", 0, DIMENSAOYOLED - 8)
            oled.show()

        elif (estadoJogoMaster == 10):
            _, valorYAnalogico = leituraAnalogico()
            posYJ2 -= (valorYAnalogico - 1) * 2
            posYJ2 = max(TAMANHOBARRAJOGADOR//2, min(DIMENSAOYOLED - TAMANHOBARRAJOGADOR//2, posYJ2))

            esp32.limparDadosParaEnviar()
            esp32.enviarDados("J2:{0}".format(posYJ2))
            atualizarFrame(int(posYJ1), posYJ2, int(posicaoBola[0]), int(posicaoBola[1]))