from machine import Pin, UART, SoftI2C
from ssd1306 import SSD1306_I2C # type: ignore
import time

# Configuração do OLED
i2c = SoftI2C(scl=Pin(15), sda=Pin(14))
oled = SSD1306_I2C(128, 64, i2c)

# Configuração da comunicação serial
uart = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1))
uart.init(bits=8, parity=None, stop=1)

# Variáveis de Estado
estadoRaspberry = 0
tempoMensagemSerial = .3
ultimaConexao = time.time()
tempoMaximoNaoRetorno = 2.0

# Limpar OLED
ultimaLinha = 0
def limparOLED():
    global ultimaLinha

    oled.fill(0)
    oled.show()
    ultimaLinha = 0

# Adicionar Nova Linha
def adicionarLinhaOLED(texto):
    global ultimaLinha

    oled.text(texto, 0, ultimaLinha * 8)
    oled.show()
    ultimaLinha += 1

# Recupear os dados da comunicação seria
# Se atingir um timeout, é retornado "timeout"
def recuperarMensagemSerial():
    global uart
    global ultimaConexao
    global tempoMaximoNaoRetorno

    while (not uart.any() and time.time() - ultimaConexao < tempoMaximoNaoRetorno):
        time.sleep(.2)

    if (not uart.any()):
        print("Sem conexao com o ESP32, tentando novamente...")
        ultimaConexao = time.time()
        uart.flush()

        return "timeout"

    if uart.any(): 
        ultimaConexao = time.time()
        buffer = uart.read()
        data = buffer.decode("ascii") 

        return data


def conectarSerial():
    global uart

    uart.write("C00")
    data = recuperarMensagemSerial()

    if (data == "C01"):
        return True

    return False


dadosDaRede = ["", ":wifi-master", ":ESP32-Master", ":123456789"]
etiquetaDadosRede = ["T", "N", "S"]
indexDadosDaRede = 0
tentativaCriacaoRede = 0

def enviarDadosDaRede():
    global estadoRaspberry
    global tempoMaximoNaoRetorno
    global dadosDaRede
    global indexDadosDaRede
    global tentativaCriacaoRede

    uart.write("C03" + dadosDaRede[indexDadosDaRede])

    data = recuperarMensagemSerial()

    if (data == "C03:OK"):
        indexDadosDaRede += 1
        tentativaCriacaoRede = 0

        if (len(dadosDaRede) == indexDadosDaRede):
            indexDadosDaRede = 0
            return True

    elif (data == "timeout"):
        estadoRaspberry = 0
    else:
        if (tentativaCriacaoRede == 5):
            estadoRaspberry = 0
        else:
            tentativaCriacaoRede += 1
    
    return False

mensagemParaSlave = False

while True:
    if (estadoRaspberry == 0):
        limparOLED()
        adicionarLinhaOLED("Conexao Serial")
        estadoRaspberry = 1

    if (estadoRaspberry == 1):
        if (conectarSerial()):
            estadoRaspberry = 2

    elif (estadoRaspberry == 2):
        adicionarLinhaOLED("Serial Conectado")
        estadoRaspberry = 3

    elif (estadoRaspberry == 3):
        uart.write("C02")
        data = recuperarMensagemSerial()

        if (data == "timeout"):
            estadoRaspberry = 0
        elif (data == "C02"):
            estadoRaspberry = 4
    
    elif (estadoRaspberry == 4):
        adicionarLinhaOLED("Enviando dados")
        adicionarLinhaOLED(" da Rede")
        estadoRaspberry = 5

    elif (estadoRaspberry == 5):
        if (enviarDadosDaRede()):
            estadoRaspberry = 6
            tempoMaximoNaoRetorno = 10.0
    
    elif (estadoRaspberry == 6):
        adicionarLinhaOLED("Criando Rede..")
        estadoRaspberry = 7

    elif (estadoRaspberry == 7):
        uart.write("C04")
        data = recuperarMensagemSerial()

        if (data == "timeout"):
            estadoRaspberry = 0

        if (data == "C04:OK"):
            tempoMaximoNaoRetorno = 2.0
            estadoRaspberry = 8

    elif (estadoRaspberry == 8):
        limparOLED()
        adicionarLinhaOLED("Rede Criada")
        adicionarLinhaOLED("")
        adicionarLinhaOLED("Dados:")
        adicionarLinhaOLED(etiquetaDadosRede[0] + dadosDaRede[1])
        adicionarLinhaOLED(etiquetaDadosRede[1] + dadosDaRede[2])
        adicionarLinhaOLED(etiquetaDadosRede[2] + dadosDaRede[3])
        estadoRaspberry = 9

    elif (estadoRaspberry == 9):
        uart.write("C05")
        data = recuperarMensagemSerial()

        if (data == "timeout"):
            estadoRaspberry = 0
        elif (data == "C05:OK"):
            estadoRaspberry = 10
    
    elif (estadoRaspberry == 10):
        adicionarLinhaOLED("Clt Conectado")
        estadoRaspberry = 11

    elif (estadoRaspberry == 11):
        
        if (mensagemParaSlave):
            uart.write("C06")
            mensagemParaSlave = False
        else:
            uart.write("C08:Mensagem do Master")
            mensagemParaSlave = True
        
        data = recuperarMensagemSerial()

        if (data == "timeout"):
            estadoRaspberry = 0
        elif (data != "C06"):
            print("Nova mensagem do Slave: " + str(data))
        

    time.sleep(tempoMensagemSerial)