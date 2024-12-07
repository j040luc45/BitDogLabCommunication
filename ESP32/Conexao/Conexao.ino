bool ledState=1; //variable used to save the state of LED

void setup() {
  Serial.begin(9600);// set baud rate to 9600
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() { 
  if(Serial.read() == 't') {
    digitalWrite(LED_BUILTIN, ledState);
    ledState=!ledState;
    Serial.print('m'); //write 'm' to the UART
  } 
}