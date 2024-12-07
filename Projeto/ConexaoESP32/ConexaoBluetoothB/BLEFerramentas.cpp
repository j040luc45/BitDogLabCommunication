#include "BLEFerramentas.h"

// -------------------------------------------------------------------------------------------------- \\
// ------------------------------- BLEDispositivosEncontradosCallbacks ------------------------------ \\
// -------------------------------------------------------------------------------------------------- \\

BLEDispositivosEncontradosCallbacks::BLEDispositivosEncontradosCallbacks(char* serverName) {
  bleServerName = serverName;
}

//Callback chamada quando outro dispositivo dispara um advertisement
void BLEDispositivosEncontradosCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
  if (advertisedDevice.getName() == bleServerName) {
    advertisedDevice.getScan()->stop();
    pServerAddress = new BLEAddress(advertisedDevice.getAddress());
    podeConectar = true;
    Serial.println("Device found. Connecting!");
  }
}

bool BLEDispositivosEncontradosCallbacks::getPodeConectar() {
  return podeConectar;
}

void BLEDispositivosEncontradosCallbacks::setPodeConectar(bool conectar) {
  podeConectar = conectar;
}

BLEAddress* BLEDispositivosEncontradosCallbacks::getPServerAddress() {
  return pServerAddress;
}

// -------------------------------------------------------------------------------------------------- \\
// --------------------------------------- BLEFerramentasSlave -------------------------------------- \\
// -------------------------------------------------------------------------------------------------- \\

BLEFerramentasSlave::BLEFerramentasSlave(char* serverName) {
  bleServerName = serverName;
}

char* BLEFerramentasSlave::getServerName() {
  return bleServerName;
}

void BLEFerramentasSlave::iniciarSlave() {
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  bleDispositivosEncontradosCallbacks = new BLEDispositivosEncontradosCallbacks(bleServerName);
  pBLEScan->setAdvertisedDeviceCallbacks(bleDispositivosEncontradosCallbacks);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

bool BLEFerramentasSlave::getPodeConectar() {
  return bleDispositivosEncontradosCallbacks->getPodeConectar();
}

void BLEFerramentasSlave::setPodeConectar(bool conectar) {
  bleDispositivosEncontradosCallbacks->setPodeConectar(conectar);
}

void BLEFerramentasSlave::slaveValorRecebidoCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  BLEFerramentasSlave::valorRecebido = (char*) pData;
  BLEFerramentasSlave::novoValorMaster = true;
}

bool BLEFerramentasSlave::conectarSlaveComServer(char * serviceUUID, char * characteristicUUIDRx, char * characteristicUUIDTx) {
  BLEClient* pClient = BLEDevice::createClient();
 
  // Conecta com o Servidor
  pClient->connect(*(bleDispositivosEncontradosCallbacks->getPServerAddress()));
 
  // Obtém o serviço do Servidor
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
    return false;
 
  // Conectando a característica RX do serviço
  masterCharacteristicRx = pRemoteService->getCharacteristic(characteristicUUIDRx);

  if (masterCharacteristicRx == nullptr)
    return false;
 
  masterCharacteristicRx->registerForNotify(slaveValorRecebidoCallback);
 
  // Conectando a característica TX do serviço
  masterCharacteristicTx = pRemoteService->getCharacteristic(characteristicUUIDTx);

  if (masterCharacteristicTx == nullptr)
    return false;
  
  serverConectado = true;
  masterCharacteristicRx->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);

  return true;
}

bool BLEFerramentasSlave::getNovoValorMaster() {
  return BLEFerramentasSlave::novoValorMaster;
}

void BLEFerramentasSlave::setNovoValorMaster(bool valor) {
  BLEFerramentasSlave::novoValorMaster = valor;
}

char* BLEFerramentasSlave::getValorRecebido() {
  return BLEFerramentasSlave::valorRecebido;
}

void BLEFerramentasSlave::enviarMensagemServer(String valor) {
  masterCharacteristicTx->writeValue(valor.c_str(), valor.length());
}