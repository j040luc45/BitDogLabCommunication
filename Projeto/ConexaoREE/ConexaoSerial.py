from machine import Pin,UART
import time

uart = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1))
uart.init(bits=8, parity=None, stop=1)

#0 - Sem conexão com o ESP32A
#1 - Conectado ao ESP32A
#2 - Passando informações da rede a ser criada
#3 - Aguardando criação da rede
#4 - Aguardando novos clientes
estadoRaspberry = 0
ultimaConexao = time.time()

def recuperarMensagemSerial():
    global uart
    global estadoRaspberry
    global ultimaConexao

    while (not uart.any() and time.time() - ultimaConexao < 2):
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

    while True:
        uart.write("C00")
        data = recuperarMensagemSerial()

        if (data == "C01"):
            estadoRaspberry = 1
            break

        time.sleep(.5)


dadosDaRede = ["", ":wifi", ":ESP32-Master", ":123456789"]
indexDadosDaRede = 0
tentativaCriacaoRede = 0

def enviarDadosDaRede():
    global estadoRaspberry
    global dadosDaRede
    global indexDadosDaRede
    global tentativaCriacaoRede

    uart.write("C03" + dadosDaRede[indexDadosDaRede])

    data = recuperarMensagemSerial()

    if (data == "C03:OK"):
        indexDadosDaRede += 1
        tentativaCriacaoRede = 0

        if (len(dadosDaRede) == indexDadosDaRede):
            estadoRaspberry = 3

    elif (data == "error"):
        estadoRaspberry = 0
    else:
        if (tentativaCriacaoRede == 5):
            estadoRaspberry = 0
        else:
            tentativaCriacaoRede += 1


while True:
    if (estadoRaspberry == 0) :
        conectarSerial()
        print("Raspberry Conectado com ESP32A")

    elif (estadoRaspberry == 1):
        uart.write("C02")
        data = recuperarMensagemSerial()

        if (data == "error"):
            estadoRaspberry = 0
        elif (data == "C02"):
            estadoRaspberry = 2

    elif (estadoRaspberry == 2):
        enviarDadosDaRede()

    elif (estadoRaspberry == 3):
        uart.write("C04")
        data = recuperarMensagemSerial()

        if (data == "error"):
            estadoRaspberry = 0

    time.sleep(.5)