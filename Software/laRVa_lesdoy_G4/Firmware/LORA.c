typedef unsigned short int byte;
#define false 0
#define true 1
uint32_t LoraSx1262_pllFrequency;
uint8_t LoraSx1262_bandwidth;
uint8_t LoraSx1262_codingRate;
uint8_t LoraSx1262_spreadingFactor;
uint8_t LoraSx1262_lowDataRateOptimize;
uint32_t LoraSx1262_transmitTimeout;
uint8_t spiBuff[32];
unsigned int inReceiveMode;
#define SX1262_NSS ((volatile uint32_t)0xE0000038)
#define SX1262_DIO1 (1<<1)
#define SPI1DAT ((volatile uint32_t)0xE0000030)
#define SPI1CTRL ((volatile uint32_t)0xE0000034)
#define SPI1STA ((volatile uint32_t)0xE0000034)
#define SPI1SS ((volatile uint32_t)0xE0000038)

void _memcpy(unsigned char *destino, unsigned char *origen, unsigned int longitud)
{
	int indice;
	for(indice = 0; indice < longitud; indice++) destino[indice] = origen[indice];
}

unsigned long millis(){ return (TIMER)/(CCLK/1000);}

unsigned char SPI_transfer(unsigned char *buffer, int tamano){
	int indice;
	SPI1SS = 0b1110;
	for(indice = 0; indice < tamano; indice++){
		SPI1DAT = buffer[indice];
		_printf("%02x ", buffer[indice]);
		while(SPI1STA & 1);
		buffer[indice] = SPI1DAT;
	}
	SPI1SS = 0b1111;
	_printf("\n");
	for(indice = 0; indice < tamano; indice++) _printf("%02x ", buffer[indice]);
	_printf("\n\n");
	return SPI1DAT;
}

void digitalWrite(unsigned int REG, unsigned int dato){
	if(dato){
		REG = REG | (1<<dato);
	}else{
		while(GPIN & (1<<0));
		REG = REG & (~(1<<dato));
	}
}

unsigned int digitalRead(unsigned int REG){
	return GPIN;
}

unsigned int LoraSx1262_begin(){
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(10);
	GPOUT |= (1<<2);
	_delay_ms(10);
	GPOUT |= (1<<1);
	GPOUT &= ~(1<<0);
	unsigned int exito = LoraSx1262_sanityCheck();
	if(!exito){
		return 0;
	}
	LoraSx1262_configureRadioEssentials();
	return 1;
}

unsigned int LoraSx1262_sanityCheck(){
	digitalWrite(SX1262_NSS, 0);
	_delay_ms(10);
	spiBuff[0] = 0x1D;
	spiBuff[1] = 0x07;
	spiBuff[2] = 0x40;
	spiBuff[3] = 0x00;
	spiBuff[4] = 0x00;
	SPI_transfer(spiBuff, 5);
	digitalWrite(SX1262_NSS, 1);
	if(spiBuff[4] == 0x14){
		_printf("SanityCheck comprobado\n");
		return 1;
	}else{
		return 0;
	}
}

void LoraSx1262_configureRadioEssentials() {
	_printf("Entrando en configureRadioEssentials\n");
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x9D;
	spiBuff[1] = 0x01;
	SPI_transfer(spiBuff, 2);
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(100);
	LoraSx1262_configSetFrequency(868000000);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x8A;
	spiBuff[1] = 0x01;
	SPI_transfer(spiBuff, 2);
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(100);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x9F;
	spiBuff[1] = 0x00;
	SPI_transfer(spiBuff, 2);
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(100);
	LoraSx1262_configSetPreset(PRESET_LONGRANGE);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x95;
	spiBuff[1] = 0x04;
	spiBuff[2] = 0x07;
	spiBuff[3] = 0x00;
	spiBuff[4] = 0x01;
	SPI_transfer(spiBuff, 5);
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(100);
	digitalWrite(SX1262_NSS,0);
	spiBuff[0] = 0x8E;
	spiBuff[1] = 22;
	spiBuff[2] = 0x02;
	SPI_transfer(spiBuff, 3);
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(100);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0xA0;
	spiBuff[1] = 0x00;
	SPI_transfer(spiBuff, 2);
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(100);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x08;
	spiBuff[1] = 0x00;
	spiBuff[2] = 0x02;
	spiBuff[3] = 0xFF;
	spiBuff[4] = 0xFF;
	spiBuff[5] = 0x00;
	spiBuff[6] = 0x00;
	spiBuff[7] = 0x00;
	spiBuff[8] = 0x00;
	SPI_transfer(spiBuff, 9);
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(100);
	_printf("configureRadioEssentials comprobado\n");
}

void LoraSx1262_transmit(uint8_t *datos, int longitud_datos) {
	_printf("Entrando en transmit\n");
	if (longitud_datos > 255) { longitud_datos = 255;}
	GPOUT |= (1<<1);
	GPOUT &= ~(1<<0);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x8C;
	spiBuff[1] = 0x00;
	spiBuff[2] = 0x08;
	spiBuff[3] = 0x00;
	spiBuff[4] = longitud_datos;
	spiBuff[5] = 0x00;
	spiBuff[6] = 0x00;
	SPI_transfer(spiBuff, 7);
	digitalWrite(SX1262_NSS, 1);
	LoraSx1262_waitForRadioCommandCompletion(100);
	_printf("Primer punto de control\n");
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x0E;
	spiBuff[1] = 0x00;
	SPI_transfer(spiBuff, 2);
	uint8_t tamano = sizeof(spiBuff);
	_printf("Tercer punto de control\n");
	for (uint16_t indice = 0; indice < longitud_datos; indice += tamano) {
		if (indice + tamano > longitud_datos) { tamano = longitud_datos - indice; }
		_memcpy(spiBuff,&(datos[indice]),tamano);
		_printf("Prueba 1 %s\n", spiBuff);
		SPI_transfer(spiBuff, tamano);
	}
	_printf("Cuarto punto de control\n");
	digitalWrite(SX1262_NSS, 1);
	LoraSx1262_waitForRadioCommandCompletion(1000);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x83;
	spiBuff[1] = 0xFF;
	spiBuff[2] = 0xFF;
	spiBuff[3] = 0xFF;
	SPI_transfer(spiBuff, 4);
	digitalWrite(SX1262_NSS, 1);
	LoraSx1262_waitForRadioCommandCompletion(LoraSx1262_transmitTimeout);
	_printf("Quinto punto de control\n");
	unsigned int inReceiveMode = false;
}

unsigned int LoraSx1262_waitForRadioCommandCompletion(uint32_t tiempo_limite) {
	uint32_t tiempo_inicio = millis();
	_printf("Tiempo 1: %d\n", tiempo_inicio);
	unsigned int datos_transmitidos = false;
	while (!datos_transmitidos) {
		_delay_ms(5);
		digitalWrite(SX1262_NSS, 0);
		spiBuff[0] = 0xC0;
		spiBuff[1] = 0x00;
		SPI_transfer(spiBuff, 2);
		digitalWrite(SX1262_NSS, 1);
		uint8_t modo_chip = (spiBuff[1] >> 4) & 0x7;
		uint8_t estado_comando = (spiBuff[1] >> 1) & 0x7;
		if (estado_comando != 0 && estado_comando != 1 && estado_comando != 2) {
			_printf("Transmitido1\n");
			datos_transmitidos = true;
		}
		if (modo_chip == 0x03 || modo_chip == 0x02) {
			_printf("Transmitido2\n");
			datos_transmitidos = true;
		}
		if (millis() - tiempo_inicio >= tiempo_limite) {
			_printf("Tiemo 2 %d\n", millis());
			return false;
		}
	}
	_printf("WE did it\n");
	return true;
}

void LoraSx1262_setModeReceive() {
	if (inReceiveMode) { return; }
	GPOUT &= ~(1<<1);
	GPOUT |= (1<<0);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x8C;
	spiBuff[1] = 0x00;
	spiBuff[2] = 0x08;
	spiBuff[3] = 0x00;
	spiBuff[4] = 0xFF;
	spiBuff[5] = 0x00;
	spiBuff[6] = 0x00;
	SPI_transfer(spiBuff, 7);
	digitalWrite(SX1262_NSS, 1);
	LoraSx1262_waitForRadioCommandCompletion(100);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x82;
	spiBuff[1] = 0xFF;
	spiBuff[2] = 0xFF;
	spiBuff[3] = 0xFF;
	SPI_transfer(spiBuff, 4);
	digitalWrite(SX1262_NSS, 1);
	LoraSx1262_waitForRadioCommandCompletion(100);
	unsigned int inReceiveMode = true;
}

int LoraSx1262_lora_receive_async(uint8_t* buffer, int tamano_max) {
	LoraSx1262_setModeReceive();
	_printf("\n\n\n");
	_printf("INICIANDO RECEPCION");
	if ((GPIN & (1<<1)) == false) { return -1; }
	_printf("INICIANDO RECEPCION");
	while ((GPIN & (1<<1))) {
		digitalWrite(SX1262_NSS, 0);
		spiBuff[0] = 0x02;
		spiBuff[1] = 0xFF;
		spiBuff[2] = 0xFF;
		SPI_transfer(spiBuff, 3);
		digitalWrite(SX1262_NSS, 1);
	}
	_printf("\n LIMPIAMOS INTERRUPCIONES");
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x14;
	spiBuff[1] = 0xFF;
	spiBuff[2] = 0xFF;
	spiBuff[3] = 0xFF;
	spiBuff[4] = 0xFF;
	SPI_transfer(spiBuff, 5);
	digitalWrite(SX1262_NSS, 1);
	unsigned int rssi = -((int)spiBuff[2]) / 2;
	unsigned int snr = ((int8_t)spiBuff[3]) / 4;
	unsigned int signalRssi = -((int)spiBuff[4]) / 2;
	_printf("RSSI: %d\n", rssi);
	_printf("SNR: %d\n", snr);
	_printf("Signal_RSSI: %d\n", signalRssi);
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x13;
	spiBuff[1] = 0xFF;
	spiBuff[2] = 0xFF;
	spiBuff[3] = 0xFF;
	SPI_transfer(spiBuff, 4);
	digitalWrite(SX1262_NSS, 1);
	uint8_t longitud_payload = spiBuff[2];
	uint8_t direccion_inicio = spiBuff[3];
	if (tamano_max < longitud_payload) {longitud_payload = tamano_max;}
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x1E;
	spiBuff[1] = direccion_inicio;
	spiBuff[2] = 0x00;
	SPI_transfer(spiBuff, 3);
	SPI_transfer(buffer,longitud_payload);
	digitalWrite(SX1262_NSS, 1);
	_printf("Paquete: ");
	_printf("%d\n",buffer);
	return longitud_payload;
}

int LoraSx1262_lora_receive_blocking(uint8_t *buffer, int tamano_max, uint32_t tiempo_limite) {
	LoraSx1262_setModeReceive();
	uint32_t tiempo_inicio = millis();
	uint32_t transcurrido = tiempo_inicio;
	while (digitalRead(SX1262_DIO1) == false) {
		if (tiempo_limite > 0) {
			transcurrido = millis() - tiempo_inicio;
			if (transcurrido >= tiempo_limite) {
				return -1;
			}
		}
	}
	return LoraSx1262_lora_receive_async(buffer,tamano_max);
}

void LoraSx1262_updateRadioFrequency() {
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x86;
	spiBuff[1] = (LoraSx1262_pllFrequency >> 24) & 0xFF;
	spiBuff[2] = (LoraSx1262_pllFrequency >> 16) & 0xFF;
	spiBuff[3] = (LoraSx1262_pllFrequency >> 8) & 0xFF;
	spiBuff[4] = (LoraSx1262_pllFrequency >> 0) & 0xFF;
	SPI_transfer(spiBuff, 5);
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(100);
}

void LoraSx1262_updateModulationParameters() {
	digitalWrite(SX1262_NSS, 0);
	spiBuff[0] = 0x8B;
	spiBuff[1] = LoraSx1262_spreadingFactor;
	spiBuff[2] = LoraSx1262_bandwidth;
	spiBuff[3] = LoraSx1262_codingRate;
	spiBuff[4] = LoraSx1262_lowDataRateOptimize;
	SPI_transfer(spiBuff, 5);
	digitalWrite(SX1262_NSS, 1);
	_delay_ms(100);
	switch (LoraSx1262_spreadingFactor) {
		case 12:
			LoraSx1262_transmitTimeout = 252000;
			break;
		case 11:
			LoraSx1262_transmitTimeout = 160000;
			break;
		case 10:
			LoraSx1262_transmitTimeout = 60000;
			break;
		case 9:
			LoraSx1262_transmitTimeout = 40000;
			break;
		case 8:
			LoraSx1262_transmitTimeout = 20000;
			break;
		case 7:
			LoraSx1262_transmitTimeout = 12000;
			break;
		case 6:
			LoraSx1262_transmitTimeout = 7000;
			break;
		default:
			LoraSx1262_transmitTimeout = 5000;
			break;
	}
}

unsigned int LoraSx1262_configSetPreset(int predefinido) {
	if (predefinido == PRESET_DEFAULT) {
		LoraSx1262_bandwidth = 5;
		LoraSx1262_codingRate = 1;
		LoraSx1262_spreadingFactor = 7;
		LoraSx1262_lowDataRateOptimize = 0;
		LoraSx1262_updateModulationParameters();
		return true;
	}
	if (predefinido == PRESET_LONGRANGE) {
		LoraSx1262_bandwidth = 4;
		LoraSx1262_codingRate = 1;
		LoraSx1262_spreadingFactor = 7;
		LoraSx1262_lowDataRateOptimize = 0;
		LoraSx1262_updateModulationParameters();
		return true;
	}
	if (predefinido == PRESET_FAST) {
		LoraSx1262_bandwidth = 6;
		LoraSx1262_codingRate = 1;
		LoraSx1262_spreadingFactor = 5;
		LoraSx1262_lowDataRateOptimize = 0;
		LoraSx1262_updateModulationParameters();
		return true;
	}
	return false;
}

unsigned int LoraSx1262_configSetFrequency(long frecuencia_hz) {
	if (frecuencia_hz < 150000000 || frecuencia_hz > 960000000) { return false;}
	LoraSx1262_pllFrequency = LoraSx1262_frequencyToPLL(frecuencia_hz);
	LoraSx1262_updateRadioFrequency();
	_printf("La frecuencia ha sido fijada a: %d\n", frecuencia_hz);
	_printf("La frecuencia del PLL a: %d\n", LoraSx1262_pllFrequency);
	return true;
}

unsigned int LoraSx1262_configSetBandwidth(int ancho_banda) {
	if (ancho_banda < 0 || ancho_banda > 0x0A || ancho_banda == 7) { return false; }
	LoraSx1262_bandwidth = ancho_banda;
	LoraSx1262_updateModulationParameters();
	return true;
}

unsigned int LoraSx1262_configSetCodingRate(int tasa_codificacion) {
	if (tasa_codificacion < 1 || tasa_codificacion > 4) { return false; }
	LoraSx1262_codingRate = tasa_codificacion;
	LoraSx1262_updateModulationParameters();
	return true;
}

unsigned int LoraSx126_configSetSpreadingFactor(int factor_expansion) {
	if (factor_expansion < 5 || factor_expansion > 12) { return false; }
	LoraSx1262_lowDataRateOptimize = (factor_expansion >= 11) ? 1 : 0;
	LoraSx1262_spreadingFactor = factor_expansion;
	LoraSx1262_updateModulationParameters();
	return true;
}

uint32_t LoraSx1262_frequencyToPLL(long frecuencia_rf) {
	uint32_t cociente = frecuencia_rf / 15625UL;
	uint32_t resto = frecuencia_rf % 15625UL;
	cociente *= 16384UL;
	resto *= 16384UL;
	return cociente + (resto / 15625UL);
}
