from machine import Pin,UART
import time

uart = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1))

uart.init(bits=8, parity=None, stop=1)

while True:
    uart.write('Mensagem do Raspberry')
    
    while not uart.any():
        time.sleep(.2)

    if uart.any(): 
        buffer = uart.read()
        data = buffer.decode('ascii')  # Decodifica os bytes em string ASCII
        print(data)
        
        time.sleep(1)