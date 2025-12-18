#include <RadioLib.h>

// Definición de pines del módulo SX1262 (LAMBDA62)
// Constructor: Module(NSS, DIO1, RESET, BUSY)
SX1262 radio = new Module(10, 2, 9, 3);

// Pines para controlar el interruptor de antena (LAMBDA62 específico)
const int PIN_RX_ENABLE = 4;
const int PIN_TX_ENABLE = 5;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("Iniciando LAMBDA62 (SX1262)..."));

  // 1. Configurar los pines del RF Switch como salidas ANTES de iniciar
  // RadioLib los gestionará, pero es buena práctica definirlos.
  pinMode(PIN_RX_ENABLE, OUTPUT);
  pinMode(PIN_TX_ENABLE, OUTPUT);

  // 2. Asignar los pines de control a RadioLib
  // Esto le dice a la librería qué pines subir/bajar al transmitir o recibir.
  radio.setRfSwitchPins(PIN_RX_ENABLE, PIN_TX_ENABLE);

  // 3. Iniciar el módulo
  // Frecuencia: 868.0
  // Bandwidth: 125.0
  // Spreading Factor: 9
  // Coding Rate: 7
  // Sync Word: SX126X_SYNC_WORD_PRIVATE (0x1424)
  // Potencia: 14 dBm
  // Longitud Preámbulo: 8
  // Voltaje TCXO: 0.0  <-- ¡ESTO ES LA CLAVE! (0 = Usar Cristal / XTAL)
  int estado = radio.begin(868.0, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 14, 8, 0.0);

  if (estado == RADIOLIB_ERR_NONE) {
    Serial.println(F("¡Modulo iniciado correctamente!"));
    
    
  } else {
    Serial.print(F("Fallo al iniciar. Codigo de error: "));
    Serial.println(estado);
    while (true);
  }
}

void loop() {
  // Comprobamos si hay datos disponibles en el puerto serie
  if (Serial.available() > 0) {
    
    // Leemos la cadena hasta que se presione ENTER (\n)
    String mensaje = Serial.readStringUntil('\n');
    mensaje.trim(); // Eliminamos espacios o saltos de línea extra

    if (mensaje.length() > 0) {
      Serial.print(F("Enviando: "));
      Serial.println(mensaje);

      // Enviar el mensaje capturado por el Monitor Serie
      int estadoTx = radio.transmit(mensaje);

      if (estadoTx == RADIOLIB_ERR_NONE) {
        Serial.println(F("¡Enviado con éxito!"));
      } else {
        Serial.print(F("Error al transmitir, código: "));
        Serial.println(estadoTx);
      }
      
      Serial.println(F("------------------------------------"));
      Serial.println(F("Escribe otro mensaje:"));
    }
  }
}
