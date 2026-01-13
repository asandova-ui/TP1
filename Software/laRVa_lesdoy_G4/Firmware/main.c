
#include <stdint.h>
#include <stdlib.h>


typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef signed char  s8;
typedef signed short s16;
typedef signed int   s32;

// Parámetros de calibración
uint32_t t_fine;
uint32_t temp_adc;
uint32_t temp_comp;
uint32_t temp_scaled;
uint32_t press_comp;
uint32_t press_adc;
uint32_t hum_adc;
uint32_t hum_comp;

uint32_t par_t1;
uint32_t par_t2;
uint32_t par_t3;

uint32_t par_p1;
uint32_t par_p2;
uint32_t par_p3;
uint32_t par_p4;
uint32_t par_p5;
uint32_t par_p6;
uint32_t par_p7;
uint32_t par_p8;
uint32_t par_p9;
uint32_t par_p10;

uint32_t par_h1;
uint32_t par_h2;
uint32_t par_h3;
uint32_t par_h4;
uint32_t par_h5;
uint32_t par_h6;
uint32_t par_h7;


//-- Registros mapeados
	//UART0
#define UARTDAT0  (*(volatile uint8_t*)0xE0000080)
#define UARTSTA0  (*(volatile uint32_t*)0xE0000084)
#define UARTBAUD0 (*(volatile uint32_t*)0xE0000084)
	//UART1
#define UARTDAT1  (*(volatile uint8_t*)0xE0000090)
#define UARTSTA1  (*(volatile uint32_t*)0xE0000094)
#define UARTBAUD1 (*(volatile uint32_t*)0xE0000094)
	//UART2
#define UARTDAT2  (*(volatile uint8_t*)0xE00000A0)
#define UARTSTA2  (*(volatile uint32_t*)0xE00000A4)
#define UARTBAUD2 (*(volatile uint32_t*)0xE00000A4)
	//SPI0
#define SPIDAT0	 (*(volatile uint32_t*)0xE0000070)
#define SPICTL0	 (*(volatile uint32_t*)0xE0000074)
#define SPISTA0	 (*(volatile uint32_t*)0xE0000074)
#define SPISS0	 (*(volatile uint32_t*)0xE0000078)
	//SPI1
#define SPIDAT1	 (*(volatile uint32_t*)0xE0000060)
#define SPICTL1	 (*(volatile uint32_t*)0xE0000064)
#define SPISTA1	 (*(volatile uint32_t*)0xE0000064)
#define SPISS1	 (*(volatile uint32_t*)0xE0000068)
	//TIMER
#define MAX_COUNT (*(volatile uint32_t*)0XE0000040)	
#define TIMER (*(volatile uint32_t*)0XE0000044)	
	//GPOUT/GPIN
#define GPOUT (*(volatile uint32_t*)0XE0000030)		
#define GPIN (*(volatile uint32_t*)0XE0000034)		
	//Interrupt enable
#define IRQEN	   		(*(volatile uint32_t*)0xE0000020)
	//Interrupt vectors
#define IRQVECT0   		(*(volatile uint32_t*)0xE0000000)
#define IRQVECT1   		(*(volatile uint32_t*)0xE0000004)
#define IRQVECT2   		(*(volatile uint32_t*)0xE0000008)
#define IRQVECT3   		(*(volatile uint32_t*)0xE000000C)
#define IRQVECT4   		(*(volatile uint32_t*)0xE0000010)
#define IRQVECT5  		(*(volatile uint32_t*)0xE0000014)
#define IRQVECT6   		(*(volatile uint32_t*)0xE0000018)
#define IRQVECT7   		(*(volatile uint32_t*)0xE000001C)
// Bits GPOUT/GPIN
	//GPOUT
#define LORA_RESET 	(1<<10)	
#define L_RX 		(1<<9)	
#define L_TX	 	(1<<8)	
#define EN_5V_UP 	(1<<7)	
#define EN_5V_M4 	(1<<6)
#define EN_1V4_M4 	(1<<5)	
#define M2_ON_OFF 	(1<<4)	
#define LED3 		(1<<3)	
#define LED2 		(1<<2)	
#define LED1 		(1<<1)	
#define LED0 		(1<<0)	
	//GPIN
#define LORA_DIO1 (1<<1)
#define LORA_BUSY (1<<0)		


// Registros sensor BME
#define RESET			0x60
#define CTRL_HUM   		0x72
#define BME_STATUS      0x73
#define CTRL_MEAS       0x74

// SPISS0 bits: [1] = BME680_CS, [0] = ADC_CS (activos bajos)
#define CS_NONE()      (SPISS0 = 0b11)  // Ninguno seleccionado
#define CS_BME_LOW()   (SPISS0 = 0b01)  // BME680 seleccionado
#define CS_ADC_LOW()   (SPISS0 = 0b10)  // ADC seleccionado



void delay_loop(uint32_t val);	// (3 + 3*val) cycles
#define CCLK (18000000)
#define _delay_us(n) delay_loop((n*(CCLK/1000)-3000)/3000)
#define _delay_ms(n) delay_loop((n*(CCLK/1000)-30)/3)


void _putch(int c)
{
	while((UARTSTA0&2)==0);
	//if (c == '\n') _putch('\r');
	UARTDAT0 = c;
}

void _puts(const char *p)
{
	while (*p)
		_putch(*(p++));
}

uint8_t _getch()
{
	while((UARTSTA0&1)==0);
	return UARTDAT0;
}

uint8_t haschar() {return UARTSTA0&1;}


#define putchar(d) _putch(d)
#include "printf.c"
#include "lora_sx1262.c"
//#include "BME_decoder.c"

const static char *menutxt="\n"
"    :::     ::::::::::: :::    :::     :::     :::          ::::::::::: :::::::: ::::::::::: \n"
"  :+: :+:       :+:     :+:    :+:     :+:   :+: :+:            :+:    :+:    :+:    :+:     \n"
" +:+   +:+      +:+     +:+    +:+     +:+  +:+   +:+           +:+    +:+    +:+    +:+     \n"
"+#++:++#++:     +#+     +#+    +#+     +:+ +#++:++#++:          +#+    +#+    +:+    +#+     \n"
"+#+     +#+     +#+     +#+     +#+   +#+  +#+     +#+          +#+    +#+    +#+    +#+     \n"
"#+#     #+#     #+#     #+#      #+#+#+#   #+#     #+#          #+#    #+#    #+#    #+#     \n"
"###     ### ########### ########## ###     ###     ###      ########### ########     ###     \n"
"------------------------------------------------\n"
"\n";             

uint8_t udat[32];
volatile uint8_t rdix,wrix;

/*uint8_t _getch()
{
	uint8_t d;
	while(rdix==wrix);
	d=udat[rdix++];
	rdix&=31;
	return d;
}
                     
uint8_t haschar() {return wrix-rdix;}*/

uint32_t __attribute__((naked)) getMEPC()
{
	asm volatile(
	"	.word	0x34101573 		\n"
	"	.word   0x34151073		\n"
	"	ret						\n"
	);
}

void __attribute__((interrupt ("machine"))) irq1_handler()
{
	_printf("\nTRAP at 0x%x\n",getMEPC());
}

void __attribute__((interrupt("machine"))) timer_handler(void)
{
    (void)TIMER;     // limpia flag de IRQ del timer
    static int toggle = 0;
    if (toggle) GPOUT |= LED0;
    else        GPOUT &= ~LED0;
    toggle = !toggle;
}

void __attribute__((interrupt ("machine"))) irq2_handler()
{
	udat[wrix++]=UARTDAT0;
	wrix&=31;
}

void  __attribute__((interrupt ("machine"))) irq3_handler(){
	static uint8_t a=32;
	UARTDAT0=a;
	if (++a>=128) a=32;
}

//GPS
char tramasGPS [200];
int j = 0;
int mensajeEmpezado = 0;
volatile int gps_msg_ready = 0;

void gpsDecoder(const char *nmeaSentence);
void miMemset(char *str, char value, int size);

void  __attribute__((interrupt ("machine"))) gpsSensor(){

	if(UARTDAT1 == '$'){
		j = 0;
		mensajeEmpezado = 1;
		tramasGPS[j] = UARTDAT1;
		//_printf("%c",tramasGPS[j]);
		j++;
		}
	else if(UARTDAT1 != '\n' && mensajeEmpezado == 1){
		tramasGPS[j] = UARTDAT1;
		//_printf("%c",tramasGPS[j]);
		j++;
	}
	else if(UARTDAT1 == '\n' && mensajeEmpezado == 1){
		tramasGPS[j]='\0';
		//_printf("%s\n", tramasGPS);
		gps_msg_ready = 1;
		mensajeEmpezado = 0;
	}
}

// --------------------------------------------------------

uint32_t spixfer0 (uint32_t d)
{
	SPIDAT0=d;
	while(SPISTA0&1);
	return SPIDAT0;
}

uint32_t spixfer1 (uint32_t d)
{
	SPIDAT1=d;
	while(SPISTA1&1);
	return SPIDAT1;
}

// --------------------------------------------------------
 

#define BAUD 115200
#define NULL ((void *)0)

uint32_t getw()
{
	uint32_t i;
	i=_getch();
	i|=_getch()<<8;
	i|=_getch()<<16;
	i|=_getch()<<24;
	return i;
}

uint8_t *_memcpy(uint8_t *pdst, uint8_t *psrc, uint32_t nb)
{
	if (nb) do {*pdst++=*psrc++; } while (--nb);
	return pdst;
}

//---------------------SENSOR GPS--------------------------//

void miMemset(char *str, char value, int size){
	for(int i=0; i<size; i++){
		str[i]=value;
	}
}

char *mistrtok(char *str, const char delimiter) {
    static char *input = NULL; // Cadena estática para mantener el estado entre llamadas
    if (str != NULL) {
        input = str; // Asignar la nueva cadena
    }
    if (input == NULL) {
        return NULL; // No hay más tokens
    }

    // Encontrar el inicio del próximo token
    char *start = input;
    while (*input != '\0' && *input != delimiter) {
        input++;
    }

    // Si encontramos un delimitador, reemplazarlo por '\0'
    if (*input == delimiter) {
        *input = '\0';
        input++; // Mover el puntero al siguiente carácter
    } else {
        input = NULL; // No hay más tokens
    }

    return start;
}

int miatoi(const char *str) {
    int result = 0;
    int isNegative = 0;

    // Manejar números negativos
    if (*str == '-') {
        isNegative = 1;
        str++;
    }

    // Convertir dígitos
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    // Aplicar signo si es necesario
    if (isNegative) {
        result *= -1;
    }

    return result;
}


double convertToDecimal(char *coord, char direction){
	int degrees = (direction == 'N' || direction == 'S') ? 2 : 3;
    char degreesStr[4] = {0}; // Para almacenar los grados
    for (int i = 0; i < degrees; i++) {
        degreesStr[i] = coord[i];
    }

    double minutes = miatoi(coord + degrees);
    double decimal = miatoi(degreesStr) + (minutes / 60.0);

    if (direction == 'S' || direction == 'W') {
        decimal *= -1;
    }

    return decimal;
}

double latDecimal;
double lonDecimal;

void gpsDecoder(const char *nmeaSentence) {
    // Verificar si la sentencia es GNGLL
    if (nmeaSentence[0] != '$' || nmeaSentence[1] != 'G' || nmeaSentence[2] != 'N' || 
        nmeaSentence[3] != 'G' || nmeaSentence[4] != 'L' || nmeaSentence[5] != 'L') {
		//_printf("%s\n", nmeaSentence);
        return;
    }

    // Variables para almacenar datos extraídos
    char latitude[20] = {0}, longitude[20] = {0};
    char latDirection = 0, lonDirection = 0, status = 0, posMode = 0;
    char utcTime[10] = {0};
	
    // Copiar la sentencia para manipularla
    char sentenceCopy[100];
    for (int i = 0; nmeaSentence[i] != '\0'; i++) {
        sentenceCopy[i] = nmeaSentence[i];
    }
	
    // Usar mistrtok para dividir la cadena
    char *token = mistrtok(sentenceCopy, ',');
    int fieldIndex = 0;

    while (token != NULL) {
        fieldIndex++;
        switch (fieldIndex) {
            case 2: // Latitud
                for (int i = 0; token[i] != '\0'; i++) {
                    latitude[i] = token[i];
                }
                break;
            case 3: // Dirección de latitud
                latDirection = token[0];
                break;
            case 4: // Longitud
                for (int i = 0; token[i] != '\0'; i++) {
                    longitude[i] = token[i];
                }
                break;
            case 5: // Dirección de longitud
                lonDirection = token[0];
                break;
            case 6: // Hora UTC
                for (int i = 0; token[i] != '\0'; i++) {
                    utcTime[i] = token[i];
                }
                break;
            case 7: // Estado
                status = token[0];
                break;
            case 8: // Indicador del modo de posición
                posMode = token[0];
                break;
        }
		
        token = mistrtok(NULL, ',');
    }

    // Convertir latitud y longitud a formato decimal
    latDecimal = convertToDecimal(latitude, latDirection);
    lonDecimal = convertToDecimal(longitude, lonDirection);

    // Mostrar los resultados
	_printf("%s\n", nmeaSentence);
    _printf("Latitud: %d\n", latDecimal);
    _printf("Longitud: %d\n", lonDecimal);
    _printf("Hora UTC: %s\n", utcTime);
    _printf("Modo de posicion: %c\n\n", posMode);
}

// -------------------- SENSOR BME680 ---------------------
//BME_quick_start();
// Escribir en un registro en el BME680
void BME680_WriteRegister(uint8_t addr, uint8_t dato){

	CS_BME_LOW(); // Selección de dispositivo (Chip Select, CS = 0) BME_ADC = 0
	SPIDAT0 = addr & 0x7F; // WR (bit 7) = 0 y dirección (bits 6..0). Cuando se escribe MSB = 1
	while(SPISTA0 & 1); // Esperamos a que se complete la transferencia
	SPIDAT0 = dato; // Ponemos el dato
	while(SPISTA0 & 1); // Esperamos a que se complete la transferencia;
	CS_NONE(); // Deselección de dispositivo (Chip Select, CS = 1) BME_ADC = 1

}

// Leer un registro del BME860
uint8_t BME680_ReadRegister(uint8_t addr){

	uint8_t d;

	if(addr & 0x80) BME680_WriteRegister(0x73, 0x00); // Página 0, 0x80..0xFF

	CS_BME_LOW();
	SPIDAT0 = (1<<7) | addr; // WR (bit 7) = 0 y dirección (bits 6..0). Cuando se lee, MSB = 0
	while(SPISTA0 & 1); // Esperamos a que se complete la transferencia
	SPIDAT0 = 0; // Byte dummy, para recoger lectura
	while(SPISTA0 & 1); // Esperamos a que se complete la transferencia
	d = SPIDAT0;
	CS_NONE(); // BME_ADC = 1

	BME680_WriteRegister(BME_STATUS,(1<<4)); // Página 1, 0x00..0x7F
	
	return d;
}

// Inicialización básica del BME680
void BME680_Init(){
	
	// Configurar oversampling y modo
	BME680_WriteRegister(CTRL_HUM, 0b00000001); // Configurar oversampling de humedad
	BME680_WriteRegister(CTRL_MEAS, 0b01010100); // Temp 2x, Presión 16x, modo sleep

    // Página 1 de los registros
    BME680_WriteRegister(BME_STATUS,(0<<4));
	
    // Lectura del parámetro de calibración
	
    //temp_adc = (BME680_ReadRegister(0x24)>>4) | (BME680_ReadRegister(0x23)<<12) | (BME680_ReadRegister(0x22)<<4);
	//_printf("%d",temp_adc);

    // Página 0 de los registros 
    BME680_WriteRegister(BME_STATUS, (1<<4));

    // Lectura de los parámetros de calibración
    // --- Temperatura ---
	par_t1 =  (uint16_t)BME680_ReadRegister(0xEA) << 8;
	par_t1 += (uint16_t)BME680_ReadRegister(0xE9);

	par_t2 =  (uint16_t)BME680_ReadRegister(0x8B) << 8;
	par_t2 += (uint16_t)BME680_ReadRegister(0x8A);

	par_t3 = BME680_ReadRegister(0x8C);

	temp_adc  = (uint32_t)BME680_ReadRegister(0x22) << 12;
	temp_adc += (uint32_t)BME680_ReadRegister(0x23) << 4;
	temp_adc += (uint32_t)BME680_ReadRegister(0x24) >> 4;

	// --- Presión ---
	par_p1  = (uint16_t)BME680_ReadRegister(0x8F) << 8;
	par_p1 += (uint16_t)BME680_ReadRegister(0x8E);

	par_p2  = (uint16_t)BME680_ReadRegister(0x91) << 8;
	par_p2 += (uint16_t)BME680_ReadRegister(0x90);

	par_p3 = BME680_ReadRegister(0x92);

	par_p4  = (uint16_t)BME680_ReadRegister(0x95) << 8;
	par_p4 += (uint16_t)BME680_ReadRegister(0x94);

	par_p5  = (uint16_t)BME680_ReadRegister(0x97) << 8;
	par_p5 += (uint16_t)BME680_ReadRegister(0x96);

	par_p6 = BME680_ReadRegister(0x99);
	par_p7 = BME680_ReadRegister(0x98);

	par_p8  = (uint16_t)BME680_ReadRegister(0x9D) << 8;
	par_p8 += (uint16_t)BME680_ReadRegister(0x9C);

	par_p9  = (uint16_t)BME680_ReadRegister(0x9F) << 8;
	par_p9 += (uint16_t)BME680_ReadRegister(0x9E);

	par_p10 = BME680_ReadRegister(0xA0);

	press_adc  = (uint32_t)BME680_ReadRegister(0x1F) << 12;
	press_adc += (uint32_t)BME680_ReadRegister(0x20) << 4;
	press_adc += (uint32_t)BME680_ReadRegister(0x21) >> 4;

	// --- Humedad ---
	par_h1  = (uint16_t)(BME680_ReadRegister(0xE3) << 4);
	par_h1 += (uint16_t)(BME680_ReadRegister(0xE2) & 0x0F);

	par_h2  = (uint16_t)(BME680_ReadRegister(0xE1) << 4);
	par_h2 += (uint16_t)(BME680_ReadRegister(0xE2) & 0x0F);

	par_h3 = BME680_ReadRegister(0xE4);
	par_h4 = BME680_ReadRegister(0xE5);
	par_h5 = BME680_ReadRegister(0xE6);
	par_h6 = BME680_ReadRegister(0xE7);
	par_h7 = BME680_ReadRegister(0xE8);

	hum_adc  = (uint16_t)BME680_ReadRegister(0x25) << 8;
	hum_adc += (uint16_t)BME680_ReadRegister(0x26);

}
// Valor de temperatura recogido del sensor

uint32_t Temp_Value(){

    uint32_t var1, var2, var3;
	
	//temp_adc = (BME680_ReadRegister(0x24)>>4) | (BME680_ReadRegister(0x23)<<12) | (BME680_ReadRegister(0x22)<<4);
    var1 = ((int32_t)temp_adc >> 3) - ((int32_t)par_t1 << 1);  
    var2 = (var1 * (int32_t)par_t2) >> 11;  
    var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((int32_t)par_t3 << 4)) >> 14;  
    t_fine = var2 + var3;  
    temp_comp = ((t_fine * 5) + 128) >> 8; 
		
    return temp_comp;
}

uint32_t Pressure_Value(){

    uint32_t var1, var2, var3;
	uint32_t press_raw = 1.4; // Pascales

    var1 = ((int32_t)t_fine >> 1) - 64000;  
    var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)par_p6) >> 2;  
    var2 = var2 + ((var1 * (int32_t)par_p5) << 1);  
    var2 = (var2 >> 2) + ((int32_t)par_p4 << 16);  
    var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)par_p3 << 5)) >> 3) + (((int32_t)par_p2 * var1) >> 1);  
    var1 = var1 >> 18;  
    var1 = ((32768 + var1) * (int32_t)par_p1) >> 15;  

    press_comp = 1048576 - press_raw;  
    press_comp = (uint32_t)((press_comp - (var2 >> 12)) * ((uint32_t)3125));  

    if (press_comp >= (1 << 30)) press_comp = ((press_comp / (uint32_t)var1) << 1);  
    else press_comp = ((press_comp << 1) / (uint32_t)var1);  

    var1 = ((int32_t)par_p9 * (int32_t)(((press_comp >> 3) * (press_comp >> 3)) >> 13)) >> 12;  
    var2 = ((int32_t)(press_comp >> 2) * (int32_t)par_p8) >> 13;  
    var3 = ((int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) * (int32_t)par_p10) >> 17;  

    press_comp = (int32_t)(press_comp) + ((var1 + var2 + var3 + ((int32_t)par_p7 << 7)) >> 4); 

    return press_comp;
}

uint32_t Humidity_Value(uint32_t temp_comp){

    uint32_t var1, var2, var3, var4, var5, var6;

    temp_scaled = (int32_t)temp_comp;  

    var1 = (int32_t)hum_adc - (int32_t)((int32_t)par_h1 << 4) - (((temp_scaled * (int32_t)par_h3) / ((int32_t)100)) >> 1);  
    var2 = ((int32_t)par_h2 * (((temp_scaled * (int32_t)par_h4) / ((int32_t)100)) + (((temp_scaled * ((temp_scaled * (int32_t)par_h5) / ((int32_t)100))) >> 6) / ((int32_t)100)) + ((int32_t)(1 << 14)))) >> 10;  
    var3 = var1 * var2;  
    var4 = (((int32_t)par_h6 << 7) + ((temp_scaled * (int32_t)par_h7) / ((int32_t)100))) >> 4;  
    var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;  
    var6 = (var4 * var5) >> 1;  

    hum_comp = (((var3 + var6) >> 10) * ((int32_t) 1000)) >> 12; 

    return hum_comp;
}


void main()
{
	char c,buf[17];
	uint8_t *p;
	unsigned int i,j;
	int n;
	void (*pcode)();
	uint32_t *pi;
	uint16_t *ps;
	uint32_t temp_comp, press_comp, hum_comp;


	UARTBAUD0=(CCLK+BAUD/2)/BAUD -1;		
	UARTBAUD1=(CCLK+9600/2)/9600 -1;	
	UARTBAUD2=(CCLK+BAUD/8)/BAUD -1;

	SPICTL0=0x0808;
	BME680_WriteRegister(CTRL_MEAS, 0x54); // Temp 2x, Presión 16x, modo sleep
	_printf("CTRL_MEAS=0x%02x\n", BME680_ReadRegister(CTRL_MEAS) );
	
	c = UARTDAT0;		// Clear RX garbage
	c = UARTDAT1;		// Clear RX garbage

	IRQVECT0=(uint32_t)irq1_handler;
	IRQVECT1=(uint32_t)timer_handler;
	IRQVECT2=(uint32_t)irq2_handler;
	IRQVECT3=(uint32_t)irq3_handler;
	IRQVECT4=(uint32_t)gpsSensor;

	IRQEN = 0;
	IRQEN |= (1<<1);        // bit 1: TIMER
	IRQEN |= (1<<2);        // bit 2: UART0 RX
	IRQEN |= (1<<4);  		// UART1 RX (GPS)

	MAX_COUNT = CCLK/1000 - 1;  // 1 ms a 18 MHz

	asm volatile ("ecall");
	asm volatile ("ebreak");

	_puts(menutxt);
	_puts("Hola mundo\n");
	
	while (1)
	{
			_puts("Command [123dxgtq]> ");
			char cmd = _getch();
			if (cmd > 32 && cmd < 127)
				_putch(cmd);
			_puts("\n");

			switch (cmd)
			{
			case '1':
			    _puts(menutxt);
				break;
			case '2':
				IRQEN^=2;	// Toggle IRQ enable for UART TX
				_delay_ms(100);
				break;
			case '3':
				_printf("--------------- DATOS GPS --------------\n");
					if (gps_msg_ready) {
						gps_msg_ready = 0;
						gpsDecoder(tramasGPS);   // parsea y hace printf aquí
					} else {
						_printf("No hay nueva trama GPS.\n");
					}
				_printf("--------------------------------------------------\n");    
				break;
			case 'x':
				_puts("Upload APP from serial port (<crtl>-F) and execute\n");
				if(getw()!=0x66567270) break;
				p=(uint8_t *)getw();
				n=getw();
				i=getw();
				if (n) {
					do { *p++=_getch(); } while(--n);
				}

				if (i>255) {
					pcode=(void (*)())i;
					pcode();
				} 
				break;
			case 'q':
				asm volatile ("jalr zero,zero");
				break;
			case 't': // Sensor temperatura, humedad y presión
				BME680_Init();
				temp_comp = Temp_Value(temp_adc);
				hum_comp = Humidity_Value(hum_adc);
				press_comp = Pressure_Value(press_adc);

				_printf("--------------- DATOS Temp/Hum/Pres --------------\n");
				_printf("\n Temperatura obtenida: ");
				_printf("%02d.%d%c", temp_comp/100, temp_comp%100, 167);
				//_printf(BME_gettemperature(0));
				_printf("\n Presion obtenida: ");
				_printf("%02d.%d%c",press_comp/100);
				//_printf(BME_getpressure());
				_printf("\n Humedad obtenida: ");
				_printf("%02d.%03d%c", hum_comp/1000, hum_comp%1000, 37);
				//_printf(BME_gethumidity());
				_printf("\n\n");
				_printf("--------------------------------------------------\n");    
				break;
			case 'g':
				if(IRQEN != 16){
					IRQEN = 16;
				}
				else{
					IRQEN = 1;
				}
				break;
			default:
				continue;
			}
	}
}

