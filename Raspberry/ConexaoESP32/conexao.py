from machine import Pin,UART
import time

uart = UART(0, baudrate=9600, tx=Pin(16), rx=Pin(17))

uart.init(bits=8, parity=None, stop=2)

led = Pin("LED", Pin.OUT)

while True:
    uart.write('t')
    
    while not uart.any():
        time.sleep(.2)

    if uart.any(): 
        data = uart.read() 
        print(data)
        if data == b'm':
            led.toggle() 
        
        time.sleep(1)