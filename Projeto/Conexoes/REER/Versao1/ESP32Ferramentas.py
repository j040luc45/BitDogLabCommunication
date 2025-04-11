from machine import Pin, UART, SoftI2C
from ssd1306 import SSD1306_I2C # type: ignore
import time

class ESP32Ferramentas:
    def __init__(self, portas, dadosDaRede, oledEnable):
        self.estado = 0

        # Configuração do OLED
        if (oledEnable):
            i2c = SoftI2C(scl=Pin(15), sda=Pin(14))
            self.oled = SSD1306_I2C(128, 64, i2c)
            self.ultimaLinha = 0
            self.oledEnable = True
        else:
            self.oledEnable = False

        self.dadosDaRede = dadosDaRede
        self.dadosDaRede.insert(0, "")
        self.etiquetaDadosRede = ["T", "N", "S"]
        self.indexDadosDaRede = 0

        self.uart = UART(0, baudrate=115200, tx=Pin(portas[0]), rx=Pin(portas[1]))
        self.uart.init(bits=8, parity=None, stop=1)

        self.estado = 0
        self.tempoMensagemSerial = .3
        self.tempoMaximoNaoRetorno = 2.0
        self.ultimaConexao = time.time()
        self.mensagemParEnviar = []

    # Limpar OLED
    def limparOLED(self):
        if (not self.oledEnable):
            return

        self.oled.fill(0)
        self.oled.show()
        self.ultimaLinha = 0

    # Adicionar Nova Linha
    def adicionarLinhaOLED(self, texto):
        if (not self.oledEnable):
            return

        self.oled.text(texto, 0, self.ultimaLinha * 8)
        self.oled.show()
        self.ultimaLinha += 1

    # Recupear os dados da comunicação seria
    # Se atingir um timeout, é retornado "timeout"
    def recuperarMensagemSerial(self):

        while (not self.uart.any() and time.time() - self.ultimaConexao < self.tempoMaximoNaoRetorno):
            time.sleep(.2)

        if (not self.uart.any()):
            print("Sem conexao com o ESP32, tentando novamente...")
            self.ultimaConexao = time.time()
            self.uart.flush()

            return "timeout"

        if self.uart.any(): 
            self.ultimaConexao = time.time()
            buffer = self.uart.read()
            data = buffer.decode("ascii") 

            return data
        
    def conectarSerial(self):
        self.uart.write("C00")
        data = self.recuperarMensagemSerial()

        if (data == "C01"):
            return True

        return False
    
    def enviarDadosDaRede(self):

        self.uart.write("C03" + self.dadosDaRede[self.indexDadosDaRede])
        data = self.recuperarMensagemSerial()

        if (data == "C03:OK"):
            self.indexDadosDaRede += 1

            if (len(self.dadosDaRede) == self.indexDadosDaRede):
                self.indexDadosDaRede = 0
                return True

        elif (data == "timeout"):
            self.estado = 0
        
        return False
    
    def enviarDados(self, texto):
        self.mensagemParEnviar.append(texto)

    def limparDados(self):
        self.mensagemParEnviar.clear()

    def executarEtapa(self):
        if (self.dadosDaRede[1] == ":wifi-master"):
            if (self.estado == 0):
                self.limparOLED()
                self.adicionarLinhaOLED("Conexao Serial")
                self.estado = 1

            elif (self.estado == 1):
                if (self.conectarSerial()):
                    self.estado = 2

            elif (self.estado == 2):
                self.adicionarLinhaOLED("Serial Conectado")
                self.estado = 3

            elif (self.estado == 3):
                self.uart.write("C02")
                data = self.recuperarMensagemSerial()

                if (data == "timeout"):
                    self.estado = 0
                elif (data == "C02"):
                    self.estado = 4
            
            elif (self.estado == 4):
                self.adicionarLinhaOLED("Enviando dados")
                self.adicionarLinhaOLED(" da Rede")
                self.estado = 5

            elif (self.estado == 5):
                if (self.enviarDadosDaRede()):
                    self.estado = 6
                    self.tempoMaximoNaoRetorno = 10.0
            
            elif (self.estado == 6):
                self.adicionarLinhaOLED("Criando Rede..")
                self.estado = 7

            elif (self.estado == 7):
                self.uart.write("C04")
                data = self.recuperarMensagemSerial()

                if (data == "timeout"):
                    self.estado = 0

                if (data == "C04:OK"):
                    self.tempoMaximoNaoRetorno = 2.0
                    self.estado = 8

            elif (self.estado == 8):
                self.limparOLED()
                self.adicionarLinhaOLED("Rede Criada")
                self.adicionarLinhaOLED("")
                self.adicionarLinhaOLED("Dados:")
                self.adicionarLinhaOLED(self.etiquetaDadosRede[0] + self.dadosDaRede[1])
                self.adicionarLinhaOLED(self.etiquetaDadosRede[1] + self.dadosDaRede[2])
                self.adicionarLinhaOLED(self.etiquetaDadosRede[2] + self.dadosDaRede[3])
                self.estado = 9

            elif (self.estado == 9):
                self.uart.write("C05")
                data = self.recuperarMensagemSerial()

                if (data == "timeout"):
                    self.estado = 0
                elif (data == "C05:OK"):
                    self.estado = 10
            
            elif (self.estado == 10):
                self.adicionarLinhaOLED("Clt Conectado")
                self.estado = 11

            elif (self.estado == 11):
                
                if (len(self.mensagemParEnviar) == 0):
                    self.uart.write("C06")
                else:
                    self.uart.write("C08:" + self.mensagemParEnviar.pop(0))

                data = self.recuperarMensagemSerial()

                if (data == "timeout"):
                    self.estado = 0
                elif (data != "C06"):
                    print("Nova mensagem do Slave: " + str(data))
            
                return True
        
        elif (self.dadosDaRede[1] == ":wifi-slave"):
            if (self.estado == 0):
                self.limparOLED()
                self.adicionarLinhaOLED("Conexao Serial")
                self.estado = 1

            elif (self.estado == 1):
                if (self.conectarSerial()):
                    self.estado = 2

            elif (self.estado == 2):
                self.adicionarLinhaOLED("Serial Conectado")
                self.estado = 3

            elif (self.estado == 3):
                self.uart.write("C02")
                data = self.recuperarMensagemSerial()

                if (data == "timeout"):
                    self.estado = 0
                elif (data == "C02"):
                    self.estado = 4
            
            elif (self.estado == 4):
                self.adicionarLinhaOLED("Enviando dados")
                self.adicionarLinhaOLED(" da Rede")
                self.estado = 5

            elif (self.estado == 5):
                if (self.enviarDadosDaRede()):
                    self.estado = 6
            
            elif (self.estado == 6):
                self.adicionarLinhaOLED("Conectando Rede")
                self.estado = 7

            elif (self.estado == 7):
                self.uart.write("C05")
                data = self.recuperarMensagemSerial()

                if (data == "timeout"):
                    self.estado = 0
                elif (data == "C05:OK"):
                    self.estado = 8
            
            elif (self.estado == 8):
                self.adicionarLinhaOLED("Rede conectada")
                self.estado = 9
            
            elif (self.estado == 9):
                
                if (len(self.mensagemParEnviar) == 0):
                    self.uart.write("C06")
                else:
                    self.uart.write("C08:" + self.mensagemParEnviar.pop(0))

                data = self.recuperarMensagemSerial()

                if (data == "timeout"):
                    self.estado = 0
                
                elif (data != "C06"):
                    print("Mensagem recebida do master: " + str(data))

                return True

        return False
    
    def executarIntervalo(self):
        time.sleep(self.tempoMensagemSerial)