// ----------------------------------------------------------------------------
Nodo Receptor (RX) LoRa con telemetría de señal
// ----------------------------------------------------------------------------

#include <RadioLib.h>

// --- INSTANCIACIÓN DEL HARDWARE ---
// Se debe replicar el mismo mapeo de pines que en el maestro para garantizar que el bus SPI y las señales de control funcionen
SX1262 radio = new Module(10, 2, 9, 3);

// --- PINES DEL RF SWITCH ---
const int PIN_RX_ENABLE = 4;
const int PIN_TX_ENABLE = 5;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("Iniciando ESCLAVO LAMBDA62 (Modo Receptor)..."));

  // 1. PREPARACIÓN DE PINES
  pinMode(PIN_RX_ENABLE, OUTPUT);
  pinMode(PIN_TX_ENABLE, OUTPUT);

  // 2. ASIGNACIÓN A RADIOLIB. Fundamental para que la librería active la ruta de recepción en el hardware
  radio.setRfSwitchPins(PIN_RX_ENABLE, PIN_TX_ENABLE);

  // 3. CONFIGURACIÓN DEL MODEM
  // Para que exista enlace (Link), los parámetros de capa física (frec, BW, SF, CR y SyncWord) idénticos al transmisor
  // El último parámetro (0.0) desactiva el control de voltaje TCXO, usando el cristal interno.
  int estado = radio.begin(868.0, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 14, 8, 0.0);

  if (estado == RADIOLIB_ERR_NONE) {
    Serial.println(F("¡Receptor iniciado y calibrado! Escuchando el canal..."));
  } else {
    Serial.print(F("Fallo de inicialización. Error: "));
    Serial.println(estado);
    while (true); // Bloqueo de seguridad
  }
}

void loop() {
  String mensajeRecibido; // Variable para almacenar el payload (carga útil) del paquete entrante

  // --- RECEPCIÓN DE PAQUETES ---
  // Utilizamos radio.receive() en modo bloqueante.
  // El microcontrolador se detiene aquí esperando:
  // A) Que llegue un paquete válido (CRC correcto).
  // B) Que ocurra un Timeout (tiempo de espera agotado).
  int estadoRx = radio.receive(mensajeRecibido);

  if (estadoRx == RADIOLIB_ERR_NONE) {
    Serial.println(F("------------------------------------"));
    Serial.print(F("DATOS RECIBIDOS: "));
    Serial.println(mensajeRecibido);

    // --- TELEMETRÍA DEL ENLACE ---
    // RSSI (Received Signal Strength Indicator): Intensidad de la señal en dBm.
    // Valores cercanos a -120 dBm indican señal muy débil; > -60 dBm señal fuerte.
    Serial.print(F("RSSI (Potencia): "));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    // En LoRa, el SNR puede ser negativo (señal por debajo del ruido) y aun así decodificarse.
    Serial.print(F("SNR (Calidad):   "));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));
    
  } else if (estadoRx == RADIOLIB_ERR_RX_TIMEOUT) {
    // El Timeout es normal en bucles de escucha; simplemente indica que  nadie transmitió en este intervalo. El bucle se reinicia.
    // Serial.println(F("Esperando..."));
    
  } else {
    // --- ERROR DE RECEPCIÓN --- Puede ser CRC Mismatch (datos corruptos por ruido) u otros fallos internos.
    Serial.print(F("Error en recepción (posible corrupción): "));
    Serial.println(estadoRx);
  }
}
