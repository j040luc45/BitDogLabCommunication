#ifndef _lorawantool_h_
#define _lorawantool_h_ 

void setIntervaloLoRaWAN(int intervalo);
void setMensagem(char * mensagem);
int executarEtapaLoRaWAN(int estadoAtualEsp32);
void setAppEui(char * appEui);
void setDevEui(char * devEui);
void setAppKey(char * appKey);

#endif