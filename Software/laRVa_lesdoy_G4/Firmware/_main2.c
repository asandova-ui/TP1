
#include <stdint.h>
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef signed char  s8;
typedef signed short s16;
typedef signed int   s32;


//-- Registros mapeados
//----------------------------------------
// UART0
//----------------------------------------
#define UART0_DAT    (*(volatile uint8_t*)0xE0000080)   // TX/RX data
#define UART0_BAUD   (*(volatile uint32_t*)0xE0000084)  // Baud Divider (write)
#define UART0_FLAGS  (*(volatile uint32_t*)0xE0000084)  // Flags (read)

//----------------------------------------
// UART1
//----------------------------------------
#define UART1_DAT    (*(volatile uint8_t*)0xE0000090)
#define UART1_BAUD   (*(volatile uint32_t*)0xE0000094)
#define UART1_FLAGS  (*(volatile uint32_t*)0xE0000094)

//----------------------------------------
// UART2
//----------------------------------------
#define UART2_DAT    (*(volatile uint8_t*)0xE00000A0)
#define UART2_BAUD   (*(volatile uint32_t*)0xE00000A4)
#define UART2_FLAGS  (*(volatile uint32_t*)0xE00000A4)

//----------------------------------------
// SPI0
//----------------------------------------
#define SPI0_DAT     (*(volatile uint32_t*)0xE0000070)
#define SPI0_CTL     (*(volatile uint32_t*)0xE0000074)
#define SPI0_SS      (*(volatile uint32_t*)0xE0000078)

//----------------------------------------
// SPI1
//----------------------------------------
#define SPI1_DAT     (*(volatile uint32_t*)0xE0000060)
#define SPI1_CTL     (*(volatile uint32_t*)0xE0000064)
#define SPI1_SS      (*(volatile uint32_t*)0xE0000068)

//----------------------------------------
// TIMER
//----------------------------------------
#define TIMER_MAXCNT (*(volatile uint32_t*)0xE0000040)  // Write
#define TIMER_CNT    (*(volatile uint32_t*)0xE0000040)  // Read

//----------------------------------------
// GPOUT / GPIN
//----------------------------------------
#define GPOUT_WRITE  (*(volatile uint32_t*)0xE0000030)
#define GPOUT_READ   (*(volatile uint32_t*)0xE0000030)  // Leer GPOUT
#define GPIN_READ    (*(volatile uint32_t*)0xE0000034)

//----------------------------------------
// IRQ
//----------------------------------------
#define IRQ_ENABLE   (*(volatile uint32_t*)0xE0000020)

#define IRQVECT0     (*(volatile uint32_t*)0xE0000000)  // Trap
#define IRQVECT1     (*(volatile uint32_t*)0xE0000004)  // Timer
#define IRQVECT2     (*(volatile uint32_t*)0xE0000008)  // RX0
#define IRQVECT3     (*(volatile uint32_t*)0xE000000C)  // TX0
#define IRQVECT4     (*(volatile uint32_t*)0xE0000010)  // RX1
#define IRQVECT5     (*(volatile uint32_t*)0xE0000014)  // TX1
#define IRQVECT6     (*(volatile uint32_t*)0xE0000018)  // RX2
#define IRQVECT7     (*(volatile uint32_t*)0xE000001C)  // TX2

//----------------------------------------
// Delays
//----------------------------------------
void delay_loop(uint32_t val);	// (3 + 3*val) cycles
#define CCLK (18000000)
#define _delay_us(n) delay_loop((n*(CCLK/1000000)-3)/3)
#define _delay_ms(n) delay_loop((n*(CCLK/1000)-3)/3)

//----------------------------------------
// UART helpers
//----------------------------------------
void _putch(int c)
{
	while((UART0_FLAGS & 2) == 0);  // Wait for THRE
	UART0_DAT = c;
}

void _puts(const char *p)
{
	while (*p) _putch(*(p++));
}

uint8_t udat[32];
volatile uint8_t rdix = 0, wrix = 0;

uint8_t _getch()
{
	uint8_t d;
	while(rdix == wrix);
	d = udat[rdix++];
	rdix &= 31;
	return d;
}

uint8_t haschar() { return wrix - rdix; }

//----------------------------------------
// RISC-V helpers
//----------------------------------------
uint32_t __attribute__((naked)) getMEPC()
{
	asm volatile(
		"	.word 0x34101573 \n"
		"	.word 0x34151073 \n"
		"	ret \n"
	);
}

void __attribute__((interrupt ("machine"))) irq1_handler()
{
	_printf("\nTRAP at 0x%x\n", getMEPC());
}

void __attribute__((interrupt ("machine"))) irq2_handler()
{
	udat[wrix++] = UART0_DAT;
	wrix &= 31;
}

void __attribute__((interrupt ("machine"))) irq3_handler()
{
	static uint8_t a = 32;
	UART0_DAT = a;
	if (++a >= 128) a = 32;
}

//----------------------------------------
// SPI helper
//----------------------------------------
uint32_t spixfer(uint32_t d)
{
	SPI0_DAT = d;
	while(SPI0_CTL & 1);
	return SPI0_DAT;
}

//----------------------------------------
// Serial helpers
//----------------------------------------
#define BAUD 115200
#define NULL ((void *)0)

uint32_t getw()
{
	uint32_t i;
	i = _getch();
	i |= _getch() << 8;
	i |= _getch() << 16;
	i |= _getch() << 24;
	return i;
}

uint8_t *_memcpy(uint8_t *pdst, uint8_t *psrc, uint32_t nb)
{
	if(nb) do { *pdst++ = *psrc++; } while(--nb);
	return pdst;
}

//----------------------------------------
// Main program
//----------------------------------------
void main()
{
	char c, buf[17];
	uint8_t *p;
	unsigned int i, j;
	int n;
	void (*pcode)();
	uint32_t *pi;
	uint16_t *ps;

	UART0_BAUD = (CCLK + BAUD/2)/BAUD - 1;
	_delay_ms(100);
	c = UART0_DAT; // Clear RX garbage

	IRQVECT0 = (uint32_t)irq1_handler;
	IRQVECT1 = (uint32_t)irq2_handler;
	IRQVECT2 = (uint32_t)irq3_handler;

	IRQ_ENABLE = 1; // Enable UART RX IRQ

	asm volatile ("ecall");
	asm volatile ("ebreak");

	_puts("Hola mundo\n");

	while (1)
	{
		_puts("Command [123dx]> ");
		char cmd = _getch();
		if (cmd > 32 && cmd < 127) _putch(cmd);
		_puts("\n");

		switch (cmd)
		{
			case '1':
				_puts("Show menu\n");
				break;
			case '2':
				IRQ_ENABLE ^= 2; // Toggle UART TX IRQ
				_delay_ms(100);
				break;
			case 'x':
				_puts("Upload APP from serial\n");
				if(getw() != 0x66567270) break;
				p = (uint8_t *)getw();
				n = getw();
				i = getw();
				if(n) do { *p++ = _getch(); } while(--n);
				if(i > 255)
				{
					pcode = (void (*)())i;
					pcode();
				}
				break;
			case 'q':
				asm volatile ("jalr zero,zero");
			default:
				continue;
		}
	}
}