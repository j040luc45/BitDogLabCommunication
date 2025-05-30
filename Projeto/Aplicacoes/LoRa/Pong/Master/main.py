from machine import Pin, SoftI2C, ADC
from ssd1306 import SSD1306_I2C # type: ignore
import time
import random
import ESP32Ferramentas

DIMENSAOXOLED = 128
DIMENSAOYOLED = 64

TAMANHOBARRAJOGADOR = 20
DISTANCIAJOGADORPAREDE = 3

TAMANHOBOLA = 2
VELOCIDADEBOLA = 2
MAXIMAPONTUACAO = 3

# Inicializar ADC para os pinos VRx (GPIO26) e VRy (GPIO27)
adc_vrx = ADC(Pin(27))
adc_vry = ADC(Pin(26))

# Inicializar OLED
i2c = SoftI2C(scl=Pin(15), sda=Pin(14))
oled = SSD1306_I2C(DIMENSAOXOLED, DIMENSAOYOLED, i2c)
oled.fill(0)
tempoUltimaAtualziacao = time.ticks_ms()

# Inicializar Comunicação ESP32
esp32 = ESP32Ferramentas.ESP32Ferramentas([0, 1], [":lora-master", ":0xF1", ":0xF1"], True)
#esp32 = ESP32Ferramentas.ESP32Ferramentas([16, 17], [":wifi-master", ":ESP32-Master", ":123456789"], True)
novaMessagemEsp = ""

# Configurações Iniciais
posYJ1 = DIMENSAOYOLED // 2
posYJ2 = DIMENSAOYOLED // 2
posicaoBola = {"x": 64, "y": random.randint(5, DIMENSAOYOLED - 5)}
direcaoBola = {"x": 1, "y": 1}
maquinaDeEstados = 0
tempoMensagemOled = 0
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

while (True):
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
        if (novaMessagemEsp[:3] == "J2:"):
            posYJ2 = int(novaMessagemEsp[3:])
            novaMessagemEsp = ""

    if (time.ticks_ms() - tempoUltimaAtualziacao > 1000 // 30):
        tempoUltimaAtualziacao = time.ticks_ms()

        if (maquinaDeEstados == 1):
            oled.fill(0)
            oled.text("Comecando Jogo", DIMENSAOXOLED // 2 - 55, DIMENSAOYOLED // 2 - 10)
            oled.text("0 - 0", DIMENSAOXOLED // 2 - 20, DIMENSAOYOLED // 2 + 2)
            oled.show()
            esp32.enviarDados("ES:1")

            maquinaDeEstados = 2
            tempoMensagemOled = time.ticks_ms()

        elif (maquinaDeEstados == 2):
            esp32.enviarDados("ES:1")

            if (time.ticks_ms() - tempoMensagemOled > 2 * 1000):
                posicaoBola = {"x": 64, "y": random.randint(5, DIMENSAOYOLED - 5)}
                placarJogo = [0, 0]
                maquinaDeEstados = 10

        elif (maquinaDeEstados == 3):
            oled.fill(0)
            oled.text("Ponto Jogador 1", DIMENSAOXOLED // 2 - 60, DIMENSAOYOLED // 2 - 10)
            oled.text("{0} - {1}".format(placarJogo[0], placarJogo[1]), DIMENSAOXOLED // 2 - 20, DIMENSAOYOLED // 2 + 2)
            oled.show()
            esp32.enviarDados("ES:3;P1:{0};P2:{1}".format(placarJogo[0], placarJogo[1]))

            tempoMensagemOled = time.ticks_ms()
            maquinaDeEstados = 4

        elif (maquinaDeEstados == 4):
            esp32.enviarDados("ES:3;P1:{0};P2:{1}".format(placarJogo[0], placarJogo[1]))

            if (time.ticks_ms() - tempoMensagemOled > 2 * 1000):
                posicaoBola = {"x": 64, "y": random.randint(5, DIMENSAOYOLED - 5)}
                direcaoBola = {"x": -1, "y": 1}
                maquinaDeEstados = 10

        elif (maquinaDeEstados == 5):
            oled.fill(0)
            oled.text("Ponto Jogador 2", DIMENSAOXOLED // 2 - 60, DIMENSAOYOLED // 2 - 10)
            oled.text("{0} - {1}".format(placarJogo[0], placarJogo[1]), DIMENSAOXOLED // 2 - 20, DIMENSAOYOLED // 2 + 2)
            oled.show()
            esp32.enviarDados("ES:5;P1:{0};P2:{1}".format(placarJogo[0], placarJogo[1]))

            tempoMensagemOled = time.ticks_ms()
            maquinaDeEstados = 6

        elif (maquinaDeEstados == 6):
            esp32.enviarDados("ES:5;P1:{0};P2:{1}".format(placarJogo[0], placarJogo[1]))

            if (time.ticks_ms() - tempoMensagemOled > 2 * 1000):
                posicaoBola = {"x": 64, "y": random.randint(5, DIMENSAOYOLED - 5)}
                direcaoBola = {"x": 1, "y": 1}
                maquinaDeEstados = 10
        
        elif (maquinaDeEstados == 20 or maquinaDeEstados == 30):
            oled.fill(0)
            oled.text("Vencedor", DIMENSAOXOLED // 2 - 30, DIMENSAOYOLED // 2 - 10)

            if (maquinaDeEstados == 20):
                oled.text("Jogador 1", DIMENSAOXOLED // 2 - 33, DIMENSAOYOLED // 2 + 2)
                maquinaDeEstados = 21
            else:
                oled.text("Jogador 2", DIMENSAOXOLED // 2 - 33, DIMENSAOYOLED // 2 + 2)
                maquinaDeEstados = 31

            oled.text("Recomecando...", 0, DIMENSAOYOLED - 8)
            oled.show()
            esp32.enviarDados("ES:20")

            tempoMensagemOled = time.ticks_ms()

        elif (maquinaDeEstados == 21):
            esp32.enviarDados("ES:20")

            if (time.ticks_ms() - tempoMensagemOled > 2 * 5000):
                maquinaDeEstados = 1

        elif (maquinaDeEstados == 31):
            esp32.enviarDados("ES:30")

            if (time.ticks_ms() - tempoMensagemOled > 2 * 5000):
                maquinaDeEstados = 1

        elif (maquinaDeEstados == 10):
            _, valorYAnalogico = leituraAnalogico()
            posYJ1 -= (valorYAnalogico - 1) * 2
            posYJ1 = max(TAMANHOBARRAJOGADOR//2, min(DIMENSAOYOLED - TAMANHOBARRAJOGADOR//2, posYJ1))
            
            if (posicaoBola["y"] >= DIMENSAOYOLED - TAMANHOBOLA//2 and direcaoBola["y"] == 1):
                direcaoBola["y"] = -direcaoBola["y"]
            elif (posicaoBola["y"] <= 0 + TAMANHOBOLA//2 and direcaoBola["y"] == -1):
                direcaoBola["y"] = -direcaoBola["y"]

            posicaoBola["x"] += direcaoBola["x"] * VELOCIDADEBOLA
            posicaoBola["y"] += direcaoBola["y"] * VELOCIDADEBOLA

            esp32.limparDadosParaEnviar()
            esp32.enviarDados("ES:10;B:{0},{1};J1:{2}".format(posicaoBola["x"], posicaoBola["y"], posYJ1))
            atualizarFrame(posYJ1, posYJ2, posicaoBola["x"], posicaoBola["y"])

            # Possibilidade 1: Jogador 1 e Bola em sua Direção
            if (posicaoBola["x"] <= 5 + TAMANHOBOLA//2 and direcaoBola["x"] == -1):
                if (posicaoBola["y"] > posYJ1 - TAMANHOBARRAJOGADOR // 2 and posicaoBola["y"] < posYJ1 + TAMANHOBARRAJOGADOR // 2):
                    direcaoBola["x"] = -direcaoBola["x"]
            
            # Possibilidade 2: Jogador 1 Perdeu
            if (posicaoBola["x"] <= 0 + TAMANHOBOLA//2 and direcaoBola["x"] == -1):
                placarJogo[1] = placarJogo[1] + 1

                if (placarJogo[1] >= MAXIMAPONTUACAO):
                    maquinaDeEstados = 30
                else:
                    maquinaDeEstados = 5

            # Possibilidade 3: Jogador 2 e Bola em sua Direção
            if (posicaoBola["x"] >= DIMENSAOXOLED - 5 - TAMANHOBOLA//2 and direcaoBola["x"] == 1):
                if (posicaoBola["y"] > posYJ2 - TAMANHOBARRAJOGADOR // 2 and posicaoBola["y"] < posYJ2 + TAMANHOBARRAJOGADOR // 2):
                    direcaoBola["x"] = -direcaoBola["x"]

            # Possibilidade 3: Jogador 2 Perde
            if (posicaoBola["x"] >= DIMENSAOXOLED - TAMANHOBOLA//2 and direcaoBola["x"] == 1):
                placarJogo[0] = placarJogo[0] + 1

                if (placarJogo[0] >= MAXIMAPONTUACAO):
                    maquinaDeEstados = 20
                else:
                    maquinaDeEstados = 3

