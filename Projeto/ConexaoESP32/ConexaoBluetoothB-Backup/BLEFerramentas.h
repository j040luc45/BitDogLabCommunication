#ifndef BLE_FERRAMENTAS
#define BLE_FERRAMENTAS

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>

//Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

// -------------------------------------------------------------------------------------------------- \\
// ------------------------------- BLEDispositivosEncontradosCallbacks ------------------------------ \\
// -------------------------------------------------------------------------------------------------- \\

class BLEDispositivosEncontradosCallbacks: public BLEAdvertisedDeviceCallbacks {
  char* bleServerName;
  boolean podeConectar;
  BLEAddress* pServerAddress;
public:
  BLEDispositivosEncontradosCallbacks(char* serverName);
  void onResult(BLEAdvertisedDevice advertisedDevice);
  boolean getPodeConectar();
  BLEAddress* getPServerAddress();
};

// -------------------------------------------------------------------------------------------------- \\
// --------------------------------------- BLEFerramentasSlave -------------------------------------- \\
// -------------------------------------------------------------------------------------------------- \\

class BLEFerramentasSlave {
  BLEDispositivosEncontradosCallbacks* bleDispositivosEncontradosCallbacks;
  BLERemoteCharacteristic* masterCharacteristicRx;
  BLERemoteCharacteristic* masterCharacteristicTx;

  //Informações do Servidor
  char* bleServerName;
  char* masterValor;
  boolean novoValorMaster;
public:
  BLEFerramentasSlave(char* serverName);
  char* getServerName();
  void iniciarSlave();
};

#endif