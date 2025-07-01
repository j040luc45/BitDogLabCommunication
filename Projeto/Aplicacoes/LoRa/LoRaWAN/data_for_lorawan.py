from machine import Pin, SoftI2C, UART
from micropyGPS import MicropyGPS
import bme280_float as bme280
import ssd1306
import time

# Endereço i2c_ina226 do INA226
INA226_ADDR = 0x40  # Endereço padrão do INA226

# Configuração i2c_ina226 para o INA226
i2c_ina226 = SoftI2C(scl=Pin(3), sda=Pin(2))  # SCL e SDA conforme seu setup

# Configuração i2c_bme280 para o BME280
i2c_bme280 = SoftI2C(scl=Pin(3), sda=Pin(2))

#UART GPS NEO06m (GP08 e GP09)
uart = UART(1,baudrate=9600, tx=Pin(8), rx=Pin(9))

gps = MicropyGPS()

# Configuração i2c_ina226 para o OLED
oled_width = 128
oled_height = 64
oled = ssd1306.SSD1306_I2C(oled_width, oled_height, i2c_ina226)

# Definir registradores
CONFIG_REG = 0x00
SHUNT_VOLTAGE_REG = 0x01
BUS_VOLTAGE_REG = 0x02
POWER_REG = 0x03
CURRENT_REG = 0x04
CALIBRATION_REG = 0x05

# Configuração do INA226 (Exemplo básico)
def configurar_ina226():
    # Configurar o INA226 para uma leitura contínua de corrente e potência
    config = 0x4127  # Configuração: medir corrente, potência e tensão (exemplo)
    i2c_ina226.writeto_mem(INA226_ADDR, CONFIG_REG, bytearray([config >> 8, config & 0xFF]))

# Função para ler a tensão no barramento (VBUS)
def ler_tensao_bus():
    data = i2c_ina226.readfrom_mem(INA226_ADDR, BUS_VOLTAGE_REG, 2)
    bus_voltage = (data[0] << 8 | data[1]) * 1.25 / 1000  # Conversão para Volts (1.25mV/bit)
    return bus_voltage

# Função para ler a tensão no resistor de shunt
def ler_tensao_shunt():
    data = i2c_ina226.readfrom_mem(INA226_ADDR, SHUNT_VOLTAGE_REG, 2)
    raw = (data[0] << 8) | data[1]
    if raw > 32767:
        raw -= 65536  # Corrigir valor negativo (conversão de complemento de dois)
    shunt_voltage = raw * 2.5 / 1_000_000  # Volts (2.5 µV/bit)
    return shunt_voltage


# Função para ler a corrente (dependendo do valor do shunt)
def ler_corrente(shunt_resistor):
    shunt_voltage = ler_tensao_shunt()
    corrente = shunt_voltage / shunt_resistor  # I = V / R (resultado com sinal)
    corrente_mA = -corrente * 1000  # inverter sinal
    return corrente_mA


# Função para calcular a potência (P = V * I)
def calcular_potencia(v_bus, corrente):
    potencia_W = v_bus * corrente / 1000  # Potência em Watts
    potencia_mW = potencia_W * 1000  # Converter de W para mW
    return potencia_mW

# Configurar o INA226
configurar_ina226()

# Resistor de shunt (em ohms)
shunt_resistor = 0.1  # 0.1 ohms

# Função para exibir informações no OLED
def exibir_no_oled(v_bus, corrente, potencia, loc, bme280):
    oled.fill(0)  # Limpar o display
    oled.text("{:.2f}V {}".format(v_bus,loc[3]), 0, 0)  # Primeira linha: Tensão
    # Corrente com sinal
    oled.text("{:+.2f}mA {}".format(corrente, bme280[0]), 0, 8)  # Corrente (mA) com sinal
    oled.text("{:+.2f}mW".format(potencia), 0, 32)  # Potência (mW)
    #oled.text("Shunt: {:+.3f}mV".format(ler_tensao_shunt() * -1000), 0, 48)  # Tensão de Shunt (mV)
    oled.text("GPS:{:.1f},{:.1f}".format(loc[0], loc[1]), 0, 16)
    oled.text("GPS:{:.1f},{:.1f}".format(loc[0], loc[1]), 0, 24) 
    oled.show()  # Atualizar o display

def update_gps():
    if uart.any():
        linha = uart.readline()
        if linha:
            try:
                for b in linha:
                    gps.update(chr(b))
            except Exception as e:
                print('Erro ao processar linha:', e)

def mostrar_gps():
    if gps.latitude[0] != 0:
        lat = gps.latitude[0] + gps.latitude[1] / 60.0
        if gps.latitude[2] == 'S':
            lat = -lat

        lon = gps.longitude[0] + gps.longitude[1] / 60.0
        if gps.longitude[2] == 'W':
            lon = -lon

        try:
            hora = '{:02}:{:02}:{:02}'.format(gps.timestamp[0], gps.timestamp[1], gps.timestamp[2])
        except:
            hora = "Hora indisponível"

        try:
            data = '{:02}/{:02}/{:02}'.format(gps.date[2], gps.date[1], gps.date[0])
        except:
            data = "Data indisponível"

        try:
            ano = gps.date[2] + 2000
            tempo = (ano, gps.date[1], gps.date[0], int(gps.timestamp[0]), int(gps.timestamp[1]), int(gps.timestamp[2]), 0, 0)
            timestamp_ = time.mktime(tempo)
        except:
            timestamp_ = 0
        
        #(-22.81415, -47.021, 0.0, '25/06/12', '19:08:35', 1749755315)
        return lat, lon, gps.altitude, data, hora, timestamp_
    else:
        print('Aguardando fix do GPS...')

def bme280_values():
    bme = bme280.BME280(i2c=i2c_bme280)  
    return bme.values

loc = [0, 0, 0, 0, 0]

def data_lora():
    #dados GPS
    data_gps = [0, 0, 0, 0, 0]
    update_gps()
    if mostrar_gps() != None:
        data_gps = mostrar_gps()
    #dados INA226
    v_bus = ler_tensao_bus()
    corrente = ler_corrente(shunt_resistor)
    potencia = calcular_potencia(v_bus, corrente)
    #dados bme280
    bme280_data = bme280_values()
    #print("teste gps",data_gps)

    return "[{}, {}, {}, {}, {}, {}, {}]".format(data_gps[0], data_gps[1], 
                                                 data_gps[-1], v_bus, corrente, bme280_data[0], bme280_data[2])
    return {
        'v_bus': v_bus,
        'corrente': corrente,
        'potencia': potencia,
        'gps': [data_gps[0], data_gps[1], data_gps[-1]],
        'temp': bme280_data[0],
        'umid': bme280_data[2]
    }