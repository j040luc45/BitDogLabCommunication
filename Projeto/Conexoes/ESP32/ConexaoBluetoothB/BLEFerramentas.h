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
  bool podeConectar;
  BLEAddress* pServerAddress;
public:
  BLEDispositivosEncontradosCallbacks(char* serverName);
  void onResult(BLEAdvertisedDevice advertisedDevice);
  bool getPodeConectar();
  void setPodeConectar(bool conectar);
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
  bool serverConectado = false;

  static inline char* valorRecebido;
  static inline bool novoValorMaster;
public:
  BLEFerramentasSlave(char* serverName);
  char* getServerName();
  void iniciarSlave();
  bool getPodeConectar();
  void setPodeConectar(bool conectar);
  bool conectarSlaveComServer(char * serviceUUID, char * characteristicUUIDRx, char * characteristicUUIDTx);
  static void slaveValorRecebidoCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
  bool getNovoValorMaster();
  void setNovoValorMaster(bool valor);
  char* getValorRecebido();
  void enviarMensagemServer(String valor);
};

#endif