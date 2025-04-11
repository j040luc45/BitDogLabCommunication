import network
import socket
from time import sleep
import machine
import rp2
import sys

def connect():
    #Connect to WLAN
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect('Raspberry-Master', '123456789')
    while wlan.isconnected() == False:
        print('Waiting for connection...')
        sleep(1)
    print(wlan.ifconfig())


connect()