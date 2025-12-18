#include <RadioLib.h>

// --- CONFIGURACIÓN DE PINES (Igual que el maestro) ---
// Constructor: Module(NSS, DIO1, RESET, BUSY)
SX1262 radio = new Module(10, 2, 9, 3);

// Pines del interruptor de antena (LAMBDA62)
const int PIN_RX_ENABLE = 4;
const int PIN_TX_ENABLE = 5;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("Iniciando ESCLAVO LAMBDA62 (Receptor)..."));

  // 1. Configurar pines del RF Switch
  pinMode(PIN_RX_ENABLE, OUTPUT);
  pinMode(PIN_TX_ENABLE, OUTPUT);

  // 2. Asignar los pines a RadioLib
  radio.setRfSwitchPins(PIN_RX_ENABLE, PIN_TX_ENABLE);

  // 3. Iniciar el módulo
  // IMPORTANTE: Debe tener EXACTAMENTE la misma configuración que el Maestro
  // Freq: 868.0, BW: 125.0, SF: 9, CR: 7, Sync: PRIVATE, Pwr: 14, Preamble: 8, TCXO: 0.0
  int estado = radio.begin(868.0, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 14, 8, 0.0);

  if (estado == RADIOLIB_ERR_NONE) {
    Serial.println(F("¡Receptor iniciado! Esperando paquetes..."));
  } else {
    Serial.print(F("Fallo al iniciar receptor. Error: "));
    Serial.println(estado);
    while (true);
  }
}

void loop() {
  String mensajeRecibido;

  // Intentamos recibir un paquete
  // radio.receive() es una función bloqueante por defecto en RadioLib básico.
  // Esperará hasta recibir algo o hasta que ocurra un error/timeout.
  int estadoRx = radio.receive(mensajeRecibido);

  if (estadoRx == RADIOLIB_ERR_NONE) {
    // --- ¡Paquete recibido con éxito! ---
    Serial.println(F("------------------------------------"));
    Serial.print(F("Mensaje recibido: "));
    Serial.println(mensajeRecibido);

    // Datos extra de telemetría (muy útiles para saber la calidad del enlace)
    Serial.print(F("RSSI (Potencia): "));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    Serial.print(F("SNR (Calidad):   "));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));

  } else if (estadoRx == RADIOLIB_ERR_RX_TIMEOUT) {
    // Timeout significa que no llegó nada en el tiempo de espera por defecto.
    // No es un error grave, simplemente no hay nadie hablando.
    // Serial.println(F("Esperando...")); 
  } else {
    // Algún otro error (CRC, Hardware, etc.)
    Serial.print(F("Error recibiendo: "));
    Serial.println(estadoRx);
  }
}