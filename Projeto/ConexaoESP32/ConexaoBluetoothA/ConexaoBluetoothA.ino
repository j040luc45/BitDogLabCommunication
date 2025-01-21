/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-ble-server-client/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>

//Default Temperature is in Celsius
//Comment the next line for Temperature in Fahrenheit
#define temperatureCelsius

//BLE server name
#define bleServerName "ESP32Master"

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

bool deviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID_TX "ca73b3ba-39f6-4ab3-91ae-186dc9577d99"
#define "c9a89672-f59f-4f04-b57b-c438f486ce20"

BLECharacteristic masterCharacteristics(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor masterDescriptor(BLEUUID((uint16_t)0x2903));

String slaveValue;
boolean newValueSlave = false;

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      slaveValue = rxValue;
      newValueSlave = true;
    }
  }
};

void setup() {
  // Start serial communication 
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *masterService = pServer->createService(SERVICE_UUID);

  masterService->addCharacteristic(&masterCharacteristics);
  masterDescriptor.setValue("Master");
  masterCharacteristics.addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = masterService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  
  // Start the service
  masterService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    if ((millis() - lastTime) > timerDelay) {
      //Set humidity Characteristic value and notify connected client

      masterCharacteristics.setValue("Mensagem de Master");
      masterCharacteristics.notify();
      Serial.println("\nDados Enviados...");
      lastTime = millis();
    }
  }

  if (newValueSlave) {
    newValueSlave = false;
    Serial.print("Mensagem Recebida: ");
    Serial.println(slaveValue.c_str());
  }

  delay(50);
}