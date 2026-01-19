#include <RadioLib.h> 

SX1262 radio = new Module(10, 2, 9, 3);

const int PIN_RX_ENABLE = 4;
const int PIN_TX_ENABLE = 5; 

void setup() {
  Serial.begin(9600); 
  while (!Serial); 

  Serial.println(F("Iniciando LAMBDA62 (SX1262) en modo TX..."));

  pinMode(PIN_RX_ENABLE, OUTPUT);
  pinMode(PIN_TX_ENABLE, OUTPUT);

  radio.setRfSwitchPins(PIN_RX_ENABLE, PIN_TX_ENABLE);

  int estado = radio.begin(868.0, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 14, 8, 0.0);

  if (estado == RADIOLIB_ERR_NONE) {
    Serial.println(F("¡Modulo iniciado correctamente! Sistema listo."));
  } else {
    Serial.print(F("Fallo crítico al iniciar. Codigo de error: "));
    Serial.println(estado);
    while (true);
  }
}

void loop() {
  if (Serial.available() > 0) {
    
    String mensaje = Serial.readStringUntil('\n');
    mensaje.trim(); 

if (mensaje.length() > 0) {
      Serial.print(F("Enviando paquete LoRa: "));
      Serial.println(mensaje);

      int estadoTx = radio.transmit(mensaje);

      if (estadoTx == RADIOLIB_ERR_NONE) {
        Serial.println(F(" -> [TX OK] Paquete enviado con éxito."));
      } else {
        Serial.print(F(" -> [TX ERROR] Fallo en transmisión, código: "));
        Serial.println(estadoTx);
      }
      
      Serial.println(F("------------------------------------"));
      Serial.println(F("Esperando nueva entrada de usuario..."));
    }
  }
}
