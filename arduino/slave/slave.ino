#include <RadioLib.h>

SX1262 radio = new Module(10, 2, 9, 3);

const int PIN_RX_ENABLE = 4;
const int PIN_TX_ENABLE = 5;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("Iniciando ESCLAVO LAMBDA62 (Modo Receptor)..."));

 pinMode(PIN_RX_ENABLE, OUTPUT);
  pinMode(PIN_TX_ENABLE, OUTPUT);

 radio.setRfSwitchPins(PIN_RX_ENABLE, PIN_TX_ENABLE);

  int estado = radio.begin(868.0, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 14, 8, 0.0);

  if (estado == RADIOLIB_ERR_NONE) {
    Serial.println(F("¡Receptor iniciado y calibrado! Escuchando el canal..."));
  } else {
    Serial.print(F("Fallo de inicialización. Error: "));
    Serial.println(estado);
    while (true);
  }
}

void loop() {
  String mensajeRecibido; 

int estadoRx = radio.receive(mensajeRecibido);

  if (estadoRx == RADIOLIB_ERR_NONE) {
    Serial.println(F("------------------------------------"));
    Serial.print(F("DATOS RECIBIDOS: "));
    Serial.println(mensajeRecibido);

    Serial.print(F("RSSI (Potencia): "));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    Serial.print(F("SNR (Calidad):   "));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));
    
  } else if (estadoRx == RADIOLIB_ERR_RX_TIMEOUT) {
    // Serial.println(F("Esperando..."));
    
  } else {
    Serial.print(F("Error en recepción (posible corrupción): "));
    Serial.println(estadoRx);
  }
}
