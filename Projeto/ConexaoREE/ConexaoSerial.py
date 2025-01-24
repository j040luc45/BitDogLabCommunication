from machine import Pin,UART
import time

uart = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1))
uart.init(bits=8, parity=None, stop=1)

estadoRaspberry = 0
tempoMensagemSerial = .3
ultimaConexao = time.time()
tempoMaximoNaoRetorno = 2.0

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

        return "error"

    if uart.any(): 
        ultimaConexao = time.time()
        buffer = uart.read()
        data = buffer.decode("ascii") 

        return data


def conectarSerial():
    global uart
    global estadoRaspberry

    uart.write("C00")
    data = recuperarMensagemSerial()

    if (data == "C01"):
        estadoRaspberry = 1


dadosDaRede = ["", ":wifi", ":ESP32-Master", ":123456789"]
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
            estadoRaspberry = 5
            tempoMaximoNaoRetorno = 10.0
            print("Dados da Rede Enviado")

    elif (data == "error"):
        estadoRaspberry = 0
    else:
        if (tentativaCriacaoRede == 5):
            estadoRaspberry = 0
        else:
            tentativaCriacaoRede += 1


while True:
    if (estadoRaspberry == 0):
        conectarSerial()

    elif (estadoRaspberry == 1):
        print("Raspberry Conectado com ESP32A")
        estadoRaspberry = 2

    elif (estadoRaspberry == 2):
        uart.write("C02")
        data = recuperarMensagemSerial()

        if (data == "error"):
            estadoRaspberry = 0
        elif (data == "C02"):
            estadoRaspberry = 3
    
    elif (estadoRaspberry == 3):
        print("Enviando dados da Rede")
        estadoRaspberry = 4

    elif (estadoRaspberry == 4):
        enviarDadosDaRede()
    
    elif (estadoRaspberry == 5):
        print("Dados da Rede enviada, aguardando criacao da Rede")
        estadoRaspberry = 6

    elif (estadoRaspberry == 6):
        uart.write("C04")
        data = recuperarMensagemSerial()

        if (data == "error"):
            estadoRaspberry = 0

        if (data == "C04:OK"):
            tempoMaximoNaoRetorno = 2.0
            estadoRaspberry = 7

    elif (estadoRaspberry == 7):
        print("Rede Criada, Aguardando Clientes...")
        estadoRaspberry = 8

    elif (estadoRaspberry == 8):
        uart.write("C05")
        data = recuperarMensagemSerial()

    time.sleep(tempoMensagemSerial)