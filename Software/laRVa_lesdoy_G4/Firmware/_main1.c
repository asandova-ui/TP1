#include <stdint.h>


#define putchar(d) _putch(d)
#include "printf.c"

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef signed char  s8;
typedef signed short s16;
typedef signed int   s32;

//----------------------------------------
// Registros mapeados
//----------------------------------------

// UART0
#define UART0_DAT    (*(volatile uint8_t *) 0xE0000080)  // TX/RX data
#define UART0_BAUD   (*(volatile uint32_t*) 0xE0000084)  // Baud Divider (write)
#define UART0_FLAGS  (*(volatile uint32_t*) 0xE0000084)  // Flags (read)

// UART1
#define UART1_DAT    (*(volatile uint8_t *) 0xE0000090)
#define UART1_BAUD   (*(volatile uint32_t*) 0xE0000094)
#define UART1_FLAGS  (*(volatile uint32_t*) 0xE0000094)

// UART2
#define UART2_DAT    (*(volatile uint8_t *) 0xE00000A0)
#define UART2_BAUD   (*(volatile uint32_t*) 0xE00000A4)
#define UART2_FLAGS  (*(volatile uint32_t*) 0xE00000A4)

// SPI0
#define SPI0_DAT     (*(volatile uint32_t*) 0xE0000070)
#define SPI0_CTL     (*(volatile uint32_t*) 0xE0000074)
#define SPI0_SS      (*(volatile uint32_t*) 0xE0000078)

// SPI1
#define SPI1_DAT     (*(volatile uint32_t*) 0xE0000060)
#define SPI1_CTL     (*(volatile uint32_t*) 0xE0000064)
#define SPI1_SS      (*(volatile uint32_t*) 0xE0000068)

// TIMER
#define TIMER_MAXCNT (*(volatile uint32_t*) 0xE0000040)   // Write: MAX_COUNT
#define TIMER_CNT    (*(volatile uint32_t*) 0xE0000040)   // Read: TIMER

// GPOUT / GPIN
#define GPOUT_WRITE  (*(volatile uint32_t*) 0xE0000030)
#define GPOUT_READ   (*(volatile uint32_t*) 0xE0000030)
#define GPIN_READ    (*(volatile uint32_t*) 0xE0000034)

// IRQ
#define IRQ_ENABLE   (*(volatile uint32_t*) 0xE0000020)

#define IRQVECT0     (*(volatile uint32_t*) 0xE0000000)   // Trap
#define IRQVECT1     (*(volatile uint32_t*) 0xE0000004)   // Timer
#define IRQVECT2     (*(volatile uint32_t*) 0xE0000008)   // UART0 RX
#define IRQVECT3     (*(volatile uint32_t*) 0xE000000C)   // UART0 TX
#define IRQVECT4     (*(volatile uint32_t*) 0xE0000010)   // UART1 RX
#define IRQVECT5     (*(volatile uint32_t*) 0xE0000014)   // UART1 TX
#define IRQVECT6     (*(volatile uint32_t*) 0xE0000018)   // UART2 RX
#define IRQVECT7     (*(volatile uint32_t*) 0xE000001C)   // UART2 TX

//----------------------------------------
// Delays (idéntico al ejemplo)
//----------------------------------------
void delay_loop(uint32_t val);  // (3 + 3*val) cycles

#define CCLK (18000000)
#define _delay_us(n) delay_loop((n*(CCLK/1000000)-3)/3)
#define _delay_ms(n) delay_loop((n*(CCLK/1000)-3)/3)

//----------------------------------------
// UART helpers (sobre UART0 como consola)
//----------------------------------------
void _putch(int c)
{
    while ((UART0_FLAGS & 2) == 0);   // THRE en bit1
    UART0_DAT = (uint8_t)c;
}

void _puts(const char *p)
{
    while (*p) _putch(*(p++));
}

uint8_t udat[32];
volatile uint8_t rdix = 0, wrix = 0;

uint8_t _getch(void)
{
    uint8_t d;
    while (rdix == wrix);
    d = udat[rdix++];
    rdix &= 31;
    return d;
}

uint8_t haschar(void)
{
    return (uint8_t)(wrix - rdix);
}

//----------------------------------------
// RISC-V helpers
//----------------------------------------
uint32_t __attribute__((naked)) getMEPC(void)
{
    asm volatile(
        "   .word 0x34101573 \n"   // csrrw a0,mepc,zero
        "   .word 0x34151073 \n"   // csrrw zero,mepc,a0
        "   ret              \n"
    );
}

//----------------------------------------
// IRQ handlers
//----------------------------------------
void __attribute__((interrupt ("machine"))) irq_trap_handler(void)
{
    // aquí podrías imprimir usando _puts si tienes un printf mínimo
    _puts("\nTRAP at 0x");
    // impresión hexadecimal mínima de getMEPC()
    uint32_t x = getMEPC();
    for (int i = 0; i < 8; i++) {
        int nib = (x >> 28) & 0xF;
        _putch("0123456789ABCDEF"[nib]);
        x <<= 4;
    }
    _puts("\n");
}

void __attribute__((interrupt ("machine"))) irq_timer_handler(void)
{
    // leer TIMER para limpiar el flag de IRQ
    (void)TIMER_CNT;

    // ejemplo: parpadeo del LED0 en GPOUT (bit0)
    static uint32_t led = 0;
    led ^= 1;
    uint32_t v = GPOUT_READ;
    if (led) v |=  1u;
    else     v &= ~1u;
    GPOUT_WRITE = v;
}

void __attribute__((interrupt ("machine"))) irq0_rx_handler(void)
{
    udat[wrix++] = UART0_DAT;
    wrix &= 31;
}

void __attribute__((interrupt ("machine"))) irq0_tx_handler(void)
{
    static uint8_t a = 32;
    UART0_DAT = a;
    if (++a >= 128) a = 32;
}

void __attribute__((interrupt ("machine"))) irq1_rx_handler(void)
{
    udat[wrix++] = UART1_DAT;
    wrix &= 31;
}

void __attribute__((interrupt ("machine"))) irq1_tx_handler(void)
{
    static uint8_t a = 32;
    UART1_DAT = a;
    if (++a >= 128) a = 32;
}

void __attribute__((interrupt ("machine"))) irq2_rx_handler(void)
{
    udat[wrix++] = UART2_DAT;
    wrix &= 31;
}

void __attribute__((interrupt ("machine"))) irq2_tx_handler(void)
{
    static uint8_t a = 32;
    UART2_DAT = a;
    if (++a >= 128) a = 32;
}

// Si en algún momento quieres usar IRQs de UART1/2, añade aquí
// irq1_rx_handler, irq1_tx_handler, irq2_rx_handler, irq2_tx_handler
// y asigna sus direcciones a IRQVECT4..7.

//----------------------------------------
// SPI helper (SPI0 maestro sencillo)
//----------------------------------------
uint32_t spixfer(uint32_t d)
{
    SPI0_DAT = d;                 // escribir dato a enviar
    while (SPI0_CTL & 1);         // esperar mientras BUSY=1
    return SPI0_DAT;              // leer dato recibido
}

//----------------------------------------
// Serial helpers adicionales
//----------------------------------------
#define BAUD 115200
#define NULL ((void *)0)

uint32_t getw(void)
{
    uint32_t i;
    i  = (uint32_t)_getch();
    i |= (uint32_t)_getch() << 8;
    i |= (uint32_t)_getch() << 16;
    i |= (uint32_t)_getch() << 24;
    return i;
}

uint8_t *_memcpy(uint8_t *pdst, const uint8_t *psrc, uint32_t nb)
{
    if (nb) do { *pdst++ = *psrc++; } while (--nb);
    return pdst;
}

//----------------------------------------
// Main program
//----------------------------------------
void main(void)
{
    char c;
    uint8_t *p;
    unsigned int i;
    int n;
    void (*pcode)(void);

    // Configurar UART0 como consola
    UART0_BAUD = (CCLK + BAUD/2)/BAUD - 1;
  //  _delay_ms(100);
    c = UART0_DAT;   // limpiar posible basura RX

    // (Opcional) configurar UART1/UART2 si las vas a usar
    // UART1_BAUD = (CCLK + BAUD/2)/BAUD - 1;
    // UART2_BAUD = (CCLK + BAUD/2)/BAUD - 1;

    // Configurar TIMER para que genere interrupciones periódicas
    // Por ejemplo, con CCLK=18MHz y MAXCNT=1_800_000, periodo ~= 0.1s
    TIMER_MAXCNT = 1800000;

    // Inicializar vectores de interrupción
    IRQVECT0 = (uint32_t)irq_trap_handler;   // Trap
    IRQVECT1 = (uint32_t)irq_timer_handler;  // Timer
    IRQVECT2 = (uint32_t)irq0_rx_handler;    // UART0 RX
    IRQVECT3 = (uint32_t)irq0_tx_handler;    // UART0 TX
    IRQVECT4 = (uint32_t)irq1_rx_handler;    // UART1 RX
    IRQVECT5 = (uint32_t)irq1_tx_handler;    // UART1 TX
    IRQVECT6 = (uint32_t)irq2_rx_handler;    // UART2 RX
    IRQVECT7 = (uint32_t)irq2_tx_handler;    // UART2 TX

    // Habilitar interrupciones:
    // bit1 TIMER, bit2 UART0 RX, bit3 UART0 TX
    IRQ_ENABLE = (1<<1) | (1<<2);  // Timer + RX0 (por ejemplo)
/*
    _puts("\n");    
    _puts("==========================================================================\n");
    _puts("====  =====    ==  ========  ====  =====  ==========    =========        =\n");
    _puts("===    =====  ===  ========  ====  ====    ==========  =============  ====\n");
    _puts("==  ==  ====  ===  ========  ====  ===  ==  =========  =============  ====\n");
    _puts("=  ====  ===  ===  ========  ====  ==  ====  ========  ====   ======  ====\n");
    _puts("=  ====  ===  ===  ========   ==   ==  ====  ========  ===  =  =====  ====\n");
    _puts("=        ===  ===  =========  ==  ===        ========  ===  =  =====  ====\n");
    _puts("=  ====  ===  ===  =========  ==  ===  ====  ========  ===  =  =====  ====\n");
    _puts("=  ====  ===  ===  ==========    ====  ====  ========  ===  =  =====  ====\n");
    _puts("=  ====  ==    ==        =====  =====  ====  =======    ===   ======  ====\n");
    _puts("==========================================================================\n");   
    */                                                                   
    _puts("Hola mundo\n");

    while (1) {
        _puts("Command [12xq]> ");
        char cmd = _getch();
        _printf("%x,",cmd);
        if (cmd > 32 && cmd < 127) _putch(cmd);
        _puts("\n");

        switch (cmd) {
            case '1':
                _puts("Menu:\n");
                _puts("1- Desceipción del menú\n");
                break;

            case '2':
                // conmutar UART0 TX IRQ (bit3)
                if (IRQ_ENABLE & (1<<3))
                    IRQ_ENABLE &= ~(1<<3);
                else
                    IRQ_ENABLE |=  (1<<3);
                _delay_ms(100);
                break;

            case 'x':
                _puts("Upload APP from serial\n");
                if (getw() != 0x66567270) break;   // 'pRv f' mágico
                p = (uint8_t *)getw();             // destino
                n = (int)getw();                   // tamaño
                i = getw();                        // dirección de arranque
                if (n) do { *p++ = _getch(); } while (--n);
                if (i > 255) {
                    pcode = (void (*)(void))i;
                    pcode();
                }
                break;

            case 'q':
                asm volatile ("jalr zero,zero");   // reset “suave”
                break;

            default:
                continue;
        }
    }
}
