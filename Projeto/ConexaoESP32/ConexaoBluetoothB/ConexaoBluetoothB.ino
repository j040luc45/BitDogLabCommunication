/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-ble-server-client/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include "BLEFerramentas.h"

//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "ESP32Master"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID_TX "c9a89672-f59f-4f04-b57b-c438f486ce20"
#define CHARACTERISTIC_UUID_RX "ca73b3ba-39f6-4ab3-91ae-186dc9577d99"

BLEFerramentasSlave * bleFerramentasSlave;

void setup() {
  
  //Start serial communication
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

  bleFerramentasSlave = new BLEFerramentasSlave(bleServerName);
  bleFerramentasSlave->iniciarSlave();
}

void loop() {
  if (bleFerramentasSlave->getPodeConectar()) {

    if (bleFerramentasSlave->conectarSlaveComServer(SERVICE_UUID, CHARACTERISTIC_UUID_RX, CHARACTERISTIC_UUID_TX)) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    bleFerramentasSlave->setPodeConectar(false);

  }
  
  if (bleFerramentasSlave->getNovoValorMaster()){

    bleFerramentasSlave->setNovoValorMaster(false);
    Serial.print(bleFerramentasSlave->getValorRecebido());

    bleFerramentasSlave->enviarMensagemServer("Mensagem do Slave");

  }

  delay(1000); // Delay a second between loops.
}