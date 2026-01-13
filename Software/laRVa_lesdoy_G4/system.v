//--------------------------------------------------------------------
// RISC-V things 
// by Jesús Arias (2022)
//--------------------------------------------------------------------

/*
Description: 

A LaRVA RISC-V system with 16KB of internal memory  

Memory map: 
----------- 

0x00000000 to 0x00003FFF Internal RAM (with inital contents) 
0x00004000 to 0x1FFFFFFF the same internal RAM repeated each 16KB 
0x20000000 to 0xDFFFFFFF       xxxx 
0xE0000000 to 0xE00000FF  IO registers 
0xE0000100 to 0xFFFFFFFF    the same IO registers repeated each 256B 

IO register map (all registers accessed as 32-bit words): 

      address  |      WRITE        |      READ      
    -----------|-------------------|--------------- 
    0xE0000080 | UART0 TX data     |  UART0 RX data 
    0xE0000084 | UART0 Baud Divider|  UART0 flags 
    0xE0000090 | UART1 TX data     |  UART1 RX data 
    0xE0000094 | UART1 Baud Divider|  UART1 flags 
    0xE00000A0 | UART2 TX data     |  UART2 RX data 
    0xE00000A4 | UART2 Baud Divider|  UART2 flags  
	           |                   | 
    0xE0000070 | SPI0 TX data      |  SPI1 RX data 
    0xE0000074 | SPI0 Control      |  SPI1 flags 
    0xE0000078 | SPI0 Slave Select |  xxxx 
               |                   | 
    0xE0000060 | SPI1 TX data      |  SPI1 RX data 
    0xE0000064 | SPI1 Control      |  SPI1 flags 
    0xE0000068 | SPI1 Slave Select |  xxxx 
		       |                   | 
    0xE0000040 |    MAX_COUNT      |     TIMER 
           	   |                   | 
    0xE0000030 |      GPOUT        |     GPOUT 
    0xE0000034 |      GPOUT        |      GPIN 
               |                   | 
    0xE0000020 | Interrupt Enable  |  Interrupt enable 
    
    0xE0000000 | IRQ vector 0 Trap | 
    0xE0000004 | IRQ vector 1 Timer| 
    0xE0000008 | IRQ vector 2 RX0  | 
    0xE000000C | IRQ vector 3 TX0  | 
    0xE0000010 | IRQ vector 4 RX1  | 
    0xE0000014 | IRQ vector 5 TX1  | 
    0xE0000018 | IRQ vector 6 RX2  | 
    0xE000001C | IRQ vector 7 TX2  | 
       ------ 

    UART Baud Divider: Baud = Fclk / (DIVIDER+1) , with DIVIDER >=7 

    UART FLAGS:    bits 31-5  bit 4  bit 3 bit 2 bit 1 bit 0 
                     xxxx      OVE    FE    TEND  THRE   DV 

        DV:   Data Valid (RX complete if 1. Cleared reading data register) 
        THRE: TX Holding register empty (ready to write to data register if 1) 
        TEND: TX end (holding reg and shift reg both empty if 1) 
        FE:   Frame Error (Stop bit received as 0 if FE=1) 
        OVE:  Overrun Error (Character received when DV was still 1) 
        (DV and THRE can request interrupts when 1) 
    ------ 

    SPI Control:   bits 31-14  bits 13-8  bits 7-0 
                      xxxx        DLEN     DIVIDER 

        DLEN:    Data lenght (8 to 32 bits) 
        DIVIDER: SCK frequency = Fclk / (2*(DIVIDER+1)) 

    SPI Flags:     bits 31-1  bit 0 
                      xxxx     BUSY 
        BUSY:  SPI exchanging data when 1 

    SPI Slave Select: bits 31-2  bit 1   bit 0 
                         xxxx     ss1     ss0 

        ss0 : Selects the SPI slave 0 when 0 (active low) 
        ss1 : Selects the SPI slave 1 when 0 (active low) 
    ------ 

    MAX_COUNT: Holds the maximum value of the timer counter. When the timer 
        reaches this value gets reset and request an interrupt if enabled. 
        Writes to MAX_COUNT also resets the timer and its interrupt flag. 

    TIMER: the current value of the timer (incremented each clock cycle) 
        Reads also clear the interrupt flag. 
    ------ 

    GPOUT: General purpose outputs. 
// GPOUT 

#define LORA_RESET  (1<<10) 
#define L_RX        (1<< 9) 
#define L_TX        (1<< 8) 
#define EN_5V_UP    (1<< 7) 
#define EN_5V_M4    (1<< 6) 
#define EN_1V4_M4   (1<< 5) 
#define M2_ON_OFF   (1<< 4) 
#define LED3        (1<< 3) 
#define LED2        (1<< 2) 
#define LED1        (1<< 1) 
#define LED0        (1<< 0) 

//    GPIN:  General purpose inputs. 
#define LORA_DIO1   (1<< 1) 
#define LORA_BUSY   (1<< 0) 
    ------ 

    Interrupt enable:  

        bit 0: Not used 
        bit 1: Enable TIMER    interrupt if 1 
        bit 2: Enable UART0 RX interrupt if 1 
        bit 3: Enable UART0 TX interrupt if 1 
        bit 4: Enable UART1 RX interrupt if 1 
        bit 5: Enable UART1 TX interrupt if 1 
        bit 6: Enable UART2 RX interrupt if 1 
        bit 7: Enable UART2 TX interrupt if 1 
 
    Interrupt Vectors: Hold the addresses of the corresponding interrupt 
        service routines.          
*/	

`include "laRVa.v"
`include "uart.v"

module SYSTEM (
	input clk,		// Main clock input 25MHz
	input reset,	// Global reset (active high)

	input	rxd,	// UART0
	output 	txd,
	
	input	rxd1,	// UART1
	output 	txd1,
	
	input	rxd2,	// UART2
	output 	txd2,
	
	output [31:0] salida ,//GPO
	input [31:0] entrada, //GPI
	
	output lora_sck,		// SPI1
	output lora_mosi,
	input  lora_miso,	
	output lora_cs,	// Flash CS
	
	output ice_sck,		// SPI0
	output ice_mosi,
	input  ice_miso,	

	output adc_cs,
	output bme680_cs
);


wire		cclk;	// CPU clock
assign	cclk=clk;

assign salida = outreg;//GPO


///////////////////////////////////////////////////////
////////////////////////// CPU ////////////////////////
///////////////////////////////////////////////////////

wire [31:0]	ca;		// CPU Address
wire [31:0]	cdo;	// CPU Data Output
wire [3:0]	mwe;	// Memory Write Enable (4 signals, one per byte lane)
wire irq;
wire [31:2]ivector;	// Where to jump on IRQ
wire trap;			// Trap irq (to IRQ vector generator)

laRVa cpu (
		.clk     (cclk ),
		.reset   (reset),
		.addr    (ca[31:2] ),
		.wdata   (cdo  ),
		.wstrb   (mwe  ),
		.rdata   (cdi  ),
		.irq     (irq  ),
		.ivector (ivector),
		.trap    (trap)
	);

 
///////////////////////////////////////////////////////
///// Memory mapping
wire iramcs;
wire iocs;
// Internal RAM selected in lower 512MB (0-0x1FFFFFFF)
assign iramcs = (ca[31:29]==3'b000);
// IO selected in last 512MB (0xE0000000-0xFFFFFFFF)
assign iocs   = (ca[31:29]==3'b111);

// Input bus mux
reg [31:0]cdi;	// Not a register
always@*
 casex ({iocs,iramcs})
        2'b01: cdi<=mdo; 
        2'b10: cdi<=iodo;
        default: cdi<=32'hxxxxxxxx;
 endcase

///////////////////////////////////////////////////////
//////////////////// internal memory //////////////////
///////////////////////////////////////////////////////
wire [31:0]	mdo;	// Output data
ram32	 ram0 ( .clk(~cclk), .re(iramcs), .wrlanes(iramcs?mwe:4'b0000),
			.addr(ca[13:2]), .data_read(mdo), .data_write(cdo) );

//////////////////////////////////////////////////
////////////////// Peripherals ///////////////////
//////////////////////////////////////////////////


wire uart0cs;	// UART0
wire uart1cs;	// UART1 
wire uart2cs;	// UART2 
wire spi0cs;	// SPI0	
wire spi1cs;	// SPI1	
wire timercs;	// Timer
wire gpocs;		// GPOUT 
wire gpics;		// GPIN 
wire irqcs;		// IRQEN 

assign uart0cs = iocs&(ca[7:4]==4'h8);
assign uart1cs = iocs&(ca[7:4]==4'h9);
assign uart2cs = iocs&(ca[7:4]==4'hA);

assign spi0cs  = iocs&(ca[7:4]==4'h7);
assign spi1cs  = iocs&(ca[7:4]==4'h6);

assign timercs = iocs&(ca[7:4] == 4'h4);
assign gpocs  = iocs&(ca[7:4]==4'h3);
assign gpics  = iocs&(ca[7:4]==4'h3);

assign irqcs  = iocs&(ca[7:5]==3'b000);

// Peripheral output bus mux
reg [31:0]iodo;	// Not a register
always@*
 casex (ca[7:2])
	// IRQ_ENABLE en 0xE0000020
    6'b001000: iodo <= {24'h0, irqen};

    // GPOUT / GPIN 0xE0000030 / 0xE0000034
    6'b001100: iodo <= salida;   // GPOUT
    6'b001101: iodo <= entrada;  // GPIN

    // MAX_COUNT / TIMER 0xE0000040 / 0xE0000044
    6'b010000: iodo <= max_count;
    6'b010001: iodo <= tcount;

    // SPI1 (LORA) 0xE0000060 / 0xE0000064
    6'b011000: iodo <= dout_spi_LORA;
    6'b011001: iodo <= {31'h0, lora_busy};

    // SPI0 (ICE/BME/BME680) 0xE0000070 / 0xE0000074
    6'b011100: iodo <= dout_spi_ICE;
    6'b011101: iodo <= {31'h0, ice_busy};

    // UART0 0xE0000080 / 0xE0000084
    6'b100000: iodo <= {24'h0, uart0_do};
    6'b100001: iodo <= {27'h0, ove0,fe0,tend0,thre0,dv0};

    // UART1 0xE0000090 / 0xE0000094
    6'b100100: iodo <= {24'h0, uart1_do};
    6'b100101: iodo <= {27'h0, ove1,fe1,tend1,thre1,dv1};

    // UART2 0xE00000A0 / 0xE00000A4
    6'b101000: iodo <= {24'h0, uart2_do};
    6'b101001: iodo <= {27'h0, ove2,fe2,tend2,thre2,dv2};

    default:   iodo <= 32'hxxxxxxxx;
 endcase
/////////////////////////////
// UART0

wire tend0,thre0,dv0,fe0,ove0; // Flags
wire [7:0] uart0_do;	// RX output data
wire uwrtx0;			// UART TX write
wire urd0;			// UART RX read (for flag clearing)
wire uwrbaud0;		// UART BGR write
// Register mapping
// Offset 0: write: TX Holding reg
// Offset 0: read strobe: Clear DV, OVE (also reads RX data buffer)
// Offset 1: write: BAUD divider
assign uwrtx0   = uart0cs & (~ca[2]) & mwe[0];
assign uwrbaud0 = uart0cs & ( ca[2]) & mwe[0] & mwe[1];
assign urd0     = uart0cs & (~ca[2]) & (mwe==4'b0000); // Clear DV, OVE flgas

UART_CORE #(.BAUDBITS(12)) uart0 ( .clk(cclk), .txd(txd), .rxd(rxd), 
	.d(cdo[15:0]), .wrtx(uwrtx0), .wrbaud(uwrbaud0),. rd(urd0), .q(uart0_do),
	.dv(dv0), .fe(fe0), .ove(ove0), .tend(tend0), .thre(thre0) );

// UART1

wire tend1,thre1,dv1,fe1,ove1; 	// Flags
wire [7:0] uart1_do;			// RX output data
wire uwrtx1;					// UART TX write
wire urd1;			// UART RX read (for flag clearing)
wire uwrbaud1;		// UART BGR write
// Register mapping
// Offset 0: write: TX Holding reg
// Offset 0: read strobe: Clear DV, OVE (also reads RX data buffer)
// Offset 1: write: BAUD divider
assign uwrtx1   = uart1cs & (~ca[2]) & mwe[0];
assign uwrbaud1 = uart1cs & ( ca[2]) & mwe[0] & mwe[1];
assign urd1     = uart1cs & (~ca[2]) & (mwe==4'b0000); // Clear DV, OVE flgas

UART_CORE #(.BAUDBITS(12)) uart1 ( .clk(cclk), .txd(txd1), .rxd(rxd1), 
	.d(cdo[15:0]), .wrtx(uwrtx1), .wrbaud(uwrbaud1),. rd(urd1), .q(uart1_do),
	.dv(dv1), .fe(fe1), .ove(ove1), .tend(tend1), .thre(thre1) );

// UART2

wire tend2,thre2,dv2,fe2,ove2; // Flags
wire [7:0] uart2_do;	// RX output data
wire uwrtx2;			// UART TX write
wire urd2;			// UART RX read (for flag clearing)
wire uwrbaud2;		// UART BGR write
// Register mapping
// Offset 0: write: TX Holding reg
// Offset 0: read strobe: Clear DV, OVE (also reads RX data buffer)
// Offset 1: write: BAUD divider
assign uwrtx2   = uart2cs & (~ca[2]) & mwe[0];
assign uwrbaud2 = uart2cs & ( ca[2]) & mwe[0] & mwe[1];
assign urd2     = uart2cs & (~ca[2]) & (mwe==4'b0000); // Clear DV, OVE flgas

UART_CORE #(.BAUDBITS(12)) uart2 ( .clk(cclk), .txd(txd2), .rxd(rxd2), 
	.d(cdo[15:0]), .wrtx(uwrtx2), .wrbaud(uwrbaud2),. rd(urd2), .q(uart2_do),
	.dv(dv2), .fe(fe2), .ove(ove2), .tend(tend2), .thre(thre2) );

///SPI0 ICE
wire ice_busy,wr_spi_ICE;

wire[31:0] dout_spi_ICE;
reg [31:0] din_spi_ICE;
reg [31:0] control_ICE;
reg [1:0] nss_ICE = 2'b11;

assign wr_spi_ICE  = (spi0cs&(ca[3:2]==2'b00) & (mwe==4'b1111));

always @(posedge cclk) begin
	if(wr_spi_ICE) din_spi_ICE <= cdo;
	else if (spi0cs&(ca[3:2]==2'b01) & (mwe==4'b1111)) control_ICE <= cdo;
	else if (spi0cs&(ca[3:2]==2'b10) & (mwe==4'b1111)) nss_ICE <= cdo[1:0];
end

assign ADC_cs = nss_ICE[0];
assign BME680_cs = nss_ICE[1];

SPI_master spi0(.clk(cclk),.miso(ice_miso),.wr(wr_spi_ICE),.din(cdo),.divider(control_ICE[7:0]),
.bits(control_ICE[13:8]),.sck(ice_sck),.mosi(ice_mosi),.busy(ice_busy),.dout(dout_spi_ICE));

///
///SPI1 LORA
wire lora_busy,wr_LORA;

wire[31:0] dout_spi_LORA;
reg [31:0] din_spi_LORA;
reg [5:0] control_LORA;
reg [7:0] divider_LORA;

always @(posedge cclk) begin
	if (spi1cs&(ca[3:2]==2'b00)) din_spi_LORA <= cdo;
	if (spi1cs&(ca[3:2]==2'b01))begin
		divider_LORA <= cdo[7:0];
		control_LORA <= cdo[13:8];
		
	end
end

assign wr_LORA = spi1cs&(ca[3:2]==2'b00);
assign lora_cs = spi1cs&(ca[3:2]==2'b10);

SPI_master spi1(.clk(cclk),.miso(lora_miso),.wr(wr_LORA),.din(din_spi_LORA),.divider(divider_LORA),
.bits(control_LORA),.sck(lora_sck),.mosi(lora_mosi),.busy(lora_busy),.dout(dout_spi_LORA));


//////////////GPO Output REG///////
reg [31:0] outreg;
wire [3:0] owr;


assign owr = {4{gpocs}}& mwe;

always @(posedge clk)
	begin
		outreg[31:24] <= owr[3] ? cdo[31:24] : outreg[31:24];
		outreg[23:16] <= owr[2] ? cdo[23:16] : outreg[23:16];
		outreg[15:8] <= owr[1] ? cdo[15:8] : outreg[15:8];
		outreg[7:0] <= owr[0] ? cdo[7:0] : outreg[7:0];

	end
	
///END ///



///////////////////////////////////////////
///////////////MAX_COUNT//////////////////

reg [31:0]tcount=0;
reg [31:0]max_count=32'hFFFFFFFF;
reg mc_flag=0;

always @ (posedge clk)
	begin
	
		if(timercs & (mwe==4'b1111)) begin
			max_count<=cdo[31:0];
			tcount <= 32'b0;
		end
		
		if(tcount>=max_count) 
			tcount=0;
		
		else
			tcount=tcount+1;
		
		mc_flag <= (tcount==max_count) ? 1:(timercs ? 0 : mc_flag);
	end
	
//always @(posedge clk) tcount<=tcount+1; //ANTIGUO

/*reg [31:0] MAX_COUNT_ANTERIOR,MAX_COUNT_ACTUAL;
reg mc_flag;
//assign timerrd = timercs & (~ca[2]) & (mwe==4'b0000); // Clear mc_flag
always @(posedge cclk)

begin
	if(MAX_COUNT_ACTUAL == MAX_COUNT_ANTERIOR)
		begin
			if(tcount == MAX_COUNT_ACTUAL)
				begin
					tcount <= 0;
					mc_flag <= 1;
				end
			else 
				mc_flag <= 0;
		end
	else
		begin
			mc_flag <= 0;
			tcount <= 0;
		end
end*/



//////////////////////////////////////////


//////////////////////////////////////////
//    Interrupt control

// IRQ enable reg
reg [7:0]irqen=0;		
always @(posedge cclk or posedge reset) begin
	if (reset) irqen<=0; else
	if (irqcs & (ca[7:2] == 6'b001000) & mwe[0]) irqen<=cdo[7:0];	
end

// IRQ vectors
reg [31:2]irqvect[0:7];	
integer k;	
always @(posedge cclk or posedge reset) begin
    if (reset) begin
        for (k = 0; k < 8; k = k + 1)
            irqvect[k] <= 30'h0;
    end else if (irqcs && (ca[7:2] < 6'd8) && (mwe == 4'b1111)) begin
        // 0xE0000000..0xE000001C -> índices 0..7
        irqvect[ca[4:2]] <= cdo[31:2];
    end
end
// Enabled IRQs
wire [6:0]irqpen={irqen[7]&thre2, irqen[6]&dv2, irqen[5]&thre1, irqen[4]&dv1, irqen[3]&thre0, irqen[2]&dv0, irqen[1]&mc_flag};	// pending IRQs

// Priority encoder
wire [2:0]vecn = trap      ? 3'b000 : (	// ECALL, EBREAK: highest priority
				 irqpen[0] ? 3'b001 : (	// Timer
				 irqpen[1] ? 3'b010	: (	// UART RX
				 irqpen[2] ? 3'b011 : (	// UART TX
				 irqpen[3] ? 3'b100 : (	// UART1 RX
				 irqpen[4] ? 3'b101 : ( // UART1 TX
				 irqpen[5] ? 3'b110 : (	// UART2 RX
				 irqpen[6] ? 3'b111 :	// UART2 TX
				 			 3'bxxx )))))));
assign ivector = irqvect[vecn];
assign irq = (irqpen!=0)|trap;

endmodule	// System


///////////////////////////////////////////////////////////////////
//					CONTROLADOR SPI MAESTRO
//
//  - Sólo modo SPI 0
//  - Divisor de reloj entre 2 y 512 (siempre par)
//  - Tamaño del dato variable entre 1 y 32 bits
//
///////////////////////////////////////////////////////////////////

module SPI_master(
	input  	clk,			// Reloj maestro (max 170MHz)
	input 	miso,
	input   wr,				// Write pulse (un ciclo clk min)
	input   [31:0]din,		// solo se usan los bits LSBs si bits<32
	input 	[7:0]divider,	// Fsck = Fclk/(2*(divider+1))
	input	[5:0]bits,		// Nº de bits (max=32, min=1)
	output 	sck,
	output	mosi,
	output  busy,			// Flag de ocupado
	output  [31:0]dout		// Contiene el dato recibido al final (LSBs)
);

///////// divisor de reloj //////

reg [7:0]divcnt=0;
wire zerodiv;
assign zerodiv=(divcnt==0);
always @(posedge clk) begin
	if (zerodiv | wr) divcnt<=divider;
	else divcnt<=divcnt-1;
end

//// SCK ////

reg sck=0;
wire falling;
always @(posedge clk) if (wr) sck<=1'b0; else if (zerodiv&busy) sck<=~sck;

assign falling=zerodiv&sck;

///// Contador de Bits /////

reg [5:0]bitcnt=0;
wire busy;
assign busy=~(bitcnt==0);
always @(posedge clk) begin
	if (wr) bitcnt<=bits;
	else if (falling & busy) bitcnt<=bitcnt-1;
end

///// Shifter //////

reg sin=0;	// serial input sampled @ posedge SCK
always @(posedge sck) sin<=miso;	// único FF que no es síncrono 100%

reg [31:0]shreg;
always @(posedge clk) begin
	if (wr) shreg<=din;
	else if (falling) begin
		shreg[7:0]<={shreg[6:0],sin};
		shreg[8] <=(bits>8 )&shreg[7];
		shreg[9] <=(bits>9 )&shreg[8];
		shreg[10]<=(bits>10)&shreg[9];
		shreg[11]<=(bits>11)&shreg[10];
		shreg[12]<=(bits>12)&shreg[11];
		shreg[13]<=(bits>13)&shreg[12];
		shreg[14]<=(bits>14)&shreg[13];
		shreg[15]<=(bits>15)&shreg[14];
		shreg[16]<=(bits>16)&shreg[15];
		shreg[17]<=(bits>17)&shreg[16];
		shreg[18]<=(bits>18)&shreg[17];
		shreg[19]<=(bits>19)&shreg[18];
		shreg[20]<=(bits>20)&shreg[19];
		shreg[21]<=(bits>21)&shreg[20];
		shreg[22]<=(bits>22)&shreg[21];
		shreg[23]<=(bits>23)&shreg[22];
		shreg[24]<=(bits>24)&shreg[23];
		shreg[25]<=(bits>25)&shreg[24];
		shreg[26]<=(bits>26)&shreg[25];
		shreg[27]<=(bits>27)&shreg[26];
		shreg[28]<=(bits>28)&shreg[27];
		shreg[29]<=(bits>29)&shreg[28];
		shreg[30]<=(bits>30)&shreg[29];
		shreg[31]<=(bits>31)&shreg[30];
	end
end
assign mosi=shreg[bits-1];	
assign dout=shreg;

endmodule

//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//-- 32-bit RAM Memory with independent byte-write lanes
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////

module ram32
 (	input	clk,
	input	re,
	input	[3:0]	wrlanes,
	input	[L2N_RAM_SIZE-1:0]	addr,
	output	[31:0]	data_read,
	input	[31:0] 	data_write
 );

parameter RAM_SIZE = 4096;
localparam L2N_RAM_SIZE = $clog2(RAM_SIZE);

reg [31:0] ram_array [0:RAM_SIZE-1];
reg [31:0] data_out;
        
assign data_read = data_out;
        
always @(posedge clk) begin
	if (wrlanes[0]) ram_array[addr][ 7: 0] <= data_write[ 7: 0];
	if (wrlanes[1]) ram_array[addr][15: 8] <= data_write[15: 8];
	if (wrlanes[2]) ram_array[addr][23:16] <= data_write[23:16];
	if (wrlanes[3]) ram_array[addr][31:24] <= data_write[31:24];
end

always @(posedge clk) begin
	if (re) data_out <= ram_array[addr];
end

initial begin
`ifdef SIMULATION
	$readmemh("rom.hex", ram_array);
`else
	$readmemh("rand.hex", ram_array);
`endif
end

endmodule

