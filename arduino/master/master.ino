// ----------------------------------------------------------------------------
// Nodo Transmisor (TX) LoRa con módulo LAMBDA62 (SX1262)
// ----------------------------------------------------------------------------

#include <RadioLib.h> // Librería de abstracción de hardware para transceptores RF

// --- INSTANCIACIÓN DEL HARDWARE ---
// Definimos el objeto 'radio' vinculando los pines físicos del Arduino a las señales del chip SX1262.
// Pines arduino: NSS (Chip Select) 10, DIO1 (Interrupción) 2, RESET 9, BUSY 3
SX1262 radio = new Module(10, 2, 9, 3);

// --- CONTROL DE ANTENA (RF SWITCH) ---
// El módulo LAMBDA62 no gestiona la antena internamente; requiere activar pines externos para conmutar entre modo Transmisión (TX) y Recepción (RX).
const int PIN_RX_ENABLE = 4; // Pin para activar ruta de recepción
const int PIN_TX_ENABLE = 5; // Pin para activar ruta de transmisión

void setup() {
  Serial.begin(9600); // Iniciamos comunicación serie para depuración y entrada de datos por teclado
  while (!Serial); // Espera activa hasta que el puerto serie esté listo

  Serial.println(F("Iniciando LAMBDA62 (SX1262) en modo TX..."));

  // 1. CONFIGURACIÓN DE I/O PARA EL SWITCH DE RF
  // Se definen pines RX y TX como salida antes de arrancar el módulo de radio para evitar estados indeterminados en la antena
  pinMode(PIN_RX_ENABLE, OUTPUT);
  pinMode(PIN_TX_ENABLE, OUTPUT);

  // 2. VINCULACIÓN CON LA LIBRERÍA
  // Pasamos el control de estos pines a RadioLib. La librería se encargará de poner TX en HIGH cuando llamemos a .transmit()
  radio.setRfSwitchPins(PIN_RX_ENABLE, PIN_TX_ENABLE);

  // 3. INICIALIZACIÓN Y CONFIGURACIÓN DEL MODEM LORA
  // Se establece la conexión SPI y se escriben los registros de configuración.
  // - Frecuencia: 868.0 MHz (Banda ISM Europa)
  // - Ancho de banda (BW): 125.0 kHz (Equilibrio alcance/velocidad)
  // - Spreading Factor (SF): 9 (Robusto ante ruido, velocidad media)
  // - Coding Rate (CR): 7 (4/7, alta corrección de errores)
  // - Sync Word: PRIVATE (0x1424, para evitar interferencias con redes públicas LoRaWAN)
  // - Potencia: 14 dBm (25mW, seguro para el consumo del Arduino)
  // - Preámbulo: 8 símbolos (Estándar para sincronización)
  // - TCXO: 0.0V (CRÍTICO: El valor 0.0 indica que usamos Cristal XTAL, no TCXO)
  int estado = radio.begin(868.0, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 14, 8, 0.0);

  // Verificación del estado de arranque
  if (estado == RADIOLIB_ERR_NONE) {
    Serial.println(F("¡Modulo iniciado correctamente! Sistema listo."));
  } else {
    Serial.print(F("Fallo crítico al iniciar. Codigo de error: "));
    Serial.println(estado);
    while (true);
  }
}

void loop() {
  // Polling del puerto serie: Esperamos a que el usuario introduzca datos
  if (Serial.available() > 0) {
    
    // Lectura bloqueante hasta encontrar el carácter de fin de línea
    String mensaje = Serial.readStringUntil('\n');
    mensaje.trim(); // Limpieza de caracteres no imprimibles (espacios, \r, \n)

    // Solo procedemos si el mensaje tiene contenido real
    if (mensaje.length() > 0) {
      Serial.print(F("Enviando paquete LoRa: "));
      Serial.println(mensaje);

      // --- TRANSMISIÓN ---
      // radio.transmit() es una función bloqueante que realiza todo el ciclo:
      // 1. Pone el chip en Standby.
      // 2. Carga el payload en el buffer FIFO.
      // 3. Activa el pin TX_ENABLE.
      // 4. Emite la trama RF.
      // 5. Espera la confirmación de TX_DONE.
      int estadoTx = radio.transmit(mensaje);

      // Verificación del resultado de la transmisión
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
