//--------------------------------------------------------------------
// RISC-V things 
// by Jesús Arias (2022)
//--------------------------------------------------------------------
/*
	Description:
	A LaRVA RISC-V system with 8KB of internal memory, and one UART
	
	Memory map:
	
	0x00000000 to 0x00001FFF	Internal RAM (with inital contents)
	0x00002000 to 0x1FFFFFFF	the same internal RAM repeated each 8KB
	0x20000000 to 0xDFFFFFFF       xxxx
	0xE0000000 to 0xE00000FF    IO registers
	0xE0000100 to 0xFFFFFFFF    the same IO registers repeated each 256B

	IO register map (all registers accessed as 32-bit words):
	
      address  |      WRITE        |      READ
    -----------|-------------------|---------------
    0xE0000080 | UART0 TX data      |  UART RX data
    0xE0000084 | UART0 Baud Divider |  UART flags
	           |                   |
    0xE0000060 |                   |  Cycle counter
	           |                   |
    0xE00000E0 | Interrupt Enable  |  Interrupt enable
    0xE00000F0 | IRQ vector 0 Trap |
    0xE00000F4 | IRQ vector 1 RX   |
    0xE00000F8 | IRQ vector 2 TX   |

    UART Baud Divider: Baud = Fcclk / (DIVIDER+1) , with DIVIDER >=7
    
    UART FLAGS:    bits 31-5  bit 4  bit 3 bit 2 bit 1 bit 0
                     xxxx      OVE    FE    TEND  THRE   DV
        DV:   Data Valid (RX complete if 1. Cleared reading data register)
        THRE: TX Holding register empty (ready to write to data register if 1)
        TEND: TX end (holding reg and shift reg both empty if 1)
        FE:   Frame Error (Stop bit received as 0 if FE=1)
        OVE:  Overrun Error (Character received when DV was still 1)
        (DV and THRE assert interrupt channels #4 and #5 when 1)

    Interrupt enable: Bits 1-0
        bit 0: Enable UART RX interrupt if 1
        bit 1: Enable UART TX interrupt if 1
         
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
	
	output sck,		// SPI0
	output mosi,
	input  miso,	

	output sck1,		// SPI1
	output mosi1,
	input  miso1,	

	output [31:0] gpout,
	input [31:0] gpin
);

wire		cclk;	// CPU clock
assign	cclk=clk;

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
ram32	 ram0 ( 
		.clk(~cclk),
		.re(iramcs), 
		.wrlanes(iramcs?mwe:4'b0000),
		.addr(ca[13:2]), // Máximo 4096 direcciones
		.data_read(mdo),
		.data_write(cdo)
		);

//////////////////////////////////////////////////
////////////////// Peripherals ///////////////////
//////////////////////////////////////////////////
reg [31:0]tcount=0;
always @(posedge clk) tcount<=tcount+1;

// UART CS
	// UART0
wire uartcs0;	// UART	at offset 0x00				//REVISAR
assign uartcs0 = iocs&(ca[7:4]==4'b1000); //Cambio UART0 a 0X80
	// UART1
wire uartcs1;	// UART	at offset 0x90
assign uartcs1 = iocs&(ca[7:4]==4'b1001); //Cambio UART1 a 0X90
	// UART2
wire uartcs2;	// UART	at offset 0xA0
assign uartcs2 = iocs&(ca[7:4]==4'b1010); //Cambio UART2 a 0XA0


// SPI CS
	//SPI0
wire spi0cs;		// SPI	at offset 0x70
assign spi0cs  = iocs&(ca[7:4]==4'b0111);
	//SPI1
wire spi1cs;		// SPI	at offset 0x60
assign spi1cs  = iocs&(ca[7:4]==4'b0110);

// GPOUT CS
	//GPOUT 
wire gpoutcs;
assign gpoutcs = iocs&(ca[7:4]==4'b0011);

// IRQ CS
wire irqcs;		// IRQEN at offset 0xE0
assign irqcs  = iocs&(ca[7:5]==3'b111);

// Peripheral output bus mux
reg [31:0]iodo;	// Not a register
always@* begin
 casex (ca[7:2])
		6'b100000: iodo <= {24'hx, uart0_do};   // UART0 RX
		6'b100001: iodo <= {27'hx, ove0, fe0, tend0, thre0, dv0}; // UART0 flags

		6'b100100: iodo <= {24'hx, uart1_do};  // UART1 RX
		6'b100101: iodo <= {27'hx, ove1, fe1, tend1, thre1, dv1}; // UART1 flags

		6'b101000: iodo <= {24'hx, uart2_do};  // UART2 RX
		6'b101001: iodo <= {27'hx, ove2, fe2, tend2, thre2, dv2}; // UART2 flags

		6'b010000: iodo <= timer;              // TIMER

		6'b011000: iodo <= spi1_txdata; //0xE0000060
		6'b011001: iodo <= spi1_flags; //0xE0000064

		6'b011100: iodo <= spi0_txdata; //0xE0000070
		6'b011101: iodo <= spi0_flags; //0xE0000074
		

		6'b001100: iodo <= gpout; //0xE0000030
		6'b001101: iodo <= gpin; //0xE0000034

		6'b001000: iodo <= {24'h0, irqen};   // IRQEN 0xE0000020
		default: iodo <= 32'hxxxxxxxx;
	endcase
end

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
assign uwrtx0   = uartcs0 & (~ca[2]) & mwe[0];
assign uwrbaud0 = uartcs0 & ( ca[2]) & mwe[0] & mwe[1];
assign urd0     = uartcs0 & (~ca[2]) & (mwe==4'b0000); // Clear DV, OVE flgas

UART_CORE #(.BAUDBITS(12)) uart0 ( .clk(cclk), .txd(txd), .rxd(rxd), 
	.d(cdo[15:0]), .wrtx(uwrtx0), .wrbaud(uwrbaud0),. rd(urd0), .q(uart0_do),
	.dv(dv0), .fe(fe0), .ove(ove0), .tend(tend0), .thre(thre0) );

/////////////////////////////
// UART1

wire tend1,thre1,dv1,fe1,ove1; // Flags
wire [7:0] uart1_do;	// RX output data
wire uwrtx1;			// UART TX write
wire urd1;			// UART RX read (for flag clearing)
wire uwrbaud1;		// UART BGR write
// Register mapping
// Offset 0: write: TX Holding reg
// Offset 0: read strobe: Clear DV, OVE (also reads RX data buffer)
// Offset 1: write: BAUD divider
assign uwrtx1   = uartcs1 & (~ca[2]) & mwe[0];
assign uwrbaud1 = uartcs1 & ( ca[2]) & mwe[0] & mwe[1];
assign urd1     = uartcs1 & (~ca[2]) & (mwe==4'b0000); // Clear DV, OVE flgas

UART_CORE #(.BAUDBITS(12)) uart1 ( .clk(cclk), .txd(txd1), .rxd(rxd1), 
	.d(cdo[15:0]), .wrtx(uwrtx1), .wrbaud(uwrbaud1),. rd(urd1), .q(uart1_do),
	.dv(dv1), .fe(fe1), .ove(ove1), .tend(tend1), .thre(thre1) );

/////////////////////////////
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
assign uwrtx2   = uartcs2 & (~ca[2]) & mwe[0];
assign uwrbaud2 = uartcs2 & ( ca[2]) & mwe[0] & mwe[1];
assign urd2     = uartcs2 & (~ca[2]) & (mwe==4'b0000); // Clear DV, OVE flgas

UART_CORE #(.BAUDBITS(12)) uart2 ( .clk(cclk), .txd(txd2), .rxd(rxd2), 
	.d(cdo[15:0]), .wrtx(uwrtx2), .wrbaud(uwrbaud2),. rd(urd2), .q(uart2_do),
	.dv(dv2), .fe(fe2), .ove(ove2), .tend(tend2), .thre(thre2) );

//////////////////////////////////////////
//    TIMER

reg [31:0] max_count;
reg [31:0] timer;
reg timer_irq;

always @(posedge cclk) begin
	if (iocs & (ca[7:2]==6'b010000) & |mwe) begin
		max_count <= cdo;
		timer <= 0;
		timer_irq <= 0;
	end else begin
		if(timer >= max_count) begin
			timer <= 0;
			timer_irq <= 1;
		end else begin
			timer <= timer + 1;
			timer_irq <= 0;
		end
	end
end

//////////////////////////////////////////
//    GPIN / GPOUT

localparam GPOUT_WORD = 6'd12; // 0xE0000030
localparam GPIN_WORD  = 6'd13; // 0xE0000034

wire gpout_we = iocs & gpout_select & |mwe;




//////////////////////////////////////////
//    Interrupt control

// IRQ enable reg
reg [1:0]irqen=0;
always @(posedge cclk or posedge reset) begin
	if (reset) irqen<=0; else
	if (irqcs & (~ca[4]) &mwe[0]) irqen<=cdo[1:0];
end

// IRQ vectors
reg [31:2]irqvect[0:3];
always @(posedge cclk) if (irqcs & ca[4] & (mwe==4'b1111)) irqvect[ca[3:2]]<=cdo[31:2];

// Enabled IRQs
wire [1:0]irqpen={irqen[1]&thre0, irqen[0]&dv0};	// pending IRQs

// Priority encoder
wire [1:0]vecn = trap      ? 2'b00 : (	// ECALL, EBREAK: highest priority
				  irqpen[0] ? 3'b001 : (
                  irqpen[1] ? 3'b010 : (
            	  irqpen[2] ? 3'b011 : (
                  irqpen[3] ? 3'b100 : (
                  irqpen[4] ? 3'b101 : (
                  irqpen[5] ? 3'b110 : (
                  irqpen[6] ? 3'b111 : 3'bxxx)))))));
	
assign ivector = irqvect[vecn];
assign irq = (irqpen!=0)|trap;

endmodule	// System




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

module GPIN (input clk, input[7:0] data, output reg[7:0] out);
	always @(posedge clk)
		out <= data;
endmodule

module GPOUT (input wr, input clk, input[10:0] data, output reg[10:0] out);
	always @(posedge clk)
		out <= wr ? data : out;
endmodule