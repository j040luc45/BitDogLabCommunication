from machine import Pin,UART
import time

#0 - Sem conexão com o ESP32A
#1 - Conectado ao ESP32A
estadoRaspberry = 0

ultimaConexao = time.time()

uart = UART(0, baudrate=115200, tx=Pin(16), rx=Pin(17))
uart.init(bits=8, parity=None, stop=1)

def recuperarMensagemSerial():
    global uart
    global estadoRaspberry
    global ultimaConexao

    while (not uart.any() and time.time() - ultimaConexao < 2):
        time.sleep(.2)

    if (not uart.any()):
        print("Sem conexao com o ESP32, tentando novamente...")
        estadoRaspberry = 0
        ultimaConexao = time.time()
        uart.flush()

        return "error"

    if uart.any(): 
        ultimaConexao = time.time()
        buffer = uart.read()
        data = buffer.decode('ascii') 

        return data


def conectarSerial():
    global uart
    global estadoRaspberry

    while True:
        uart.write('C00')
        
        data = recuperarMensagemSerial()

        if (data == "C01"):
            estadoRaspberry = 1
            break

        time.sleep(1)


while True:
    if (estadoRaspberry == 0) :
        conectarSerial()

        print("Raspberry Conectado com ESP32A")
        print("Começando Conexao com ESP32B")

    elif (estadoRaspberry == 1):
        data = recuperarMensagemSerial()

        if (data == "error"):
            continue

        if (data == "C03"):
            print("Raspberry Conectado com ESP32B")