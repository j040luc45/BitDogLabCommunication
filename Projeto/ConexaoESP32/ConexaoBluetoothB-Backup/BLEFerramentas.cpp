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

boolean BLEDispositivosEncontradosCallbacks::getPodeConectar() {
  return podeConectar;
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

static BLERemoteCharacteristic* masterCharacteristicRx = NULL;
static BLERemoteCharacteristic* masterCharacteristicTx = NULL;

//Informações do Servidor
char* bleServerName = "ESP32Master";
static BLEAddress *pServerAddress;

char* masterValor;
boolean novoValorMaster = false;
boolean podeConectar = false;



//Callback chamada quando o Slave recebe um novo Valor
static void slaveValorRecebidoCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  masterValor = (char*) pData;
  novoValorMaster = true;
}

void iniciarSlave() {
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new BLEDispositivosEncontradosCallbacks("Baatata"));
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

bool conectarSlaveComServer(BLEAddress pAddress, char * serviceUUID, char * characteristicUUIDRx, char * characteristicUUIDTx) {
  BLEClient* pClient = BLEDevice::createClient();
 
  // Conecta com o Servidor
  pClient->connect(pAddress);
 
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

  return true;
}