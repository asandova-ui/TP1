//--------------------------------------------------------------------
// RISC-V things
// by Jesús Arias
//--------------------------------------------------------------------
`include "system.v"
`include "pll.v"

// Top module. (signals assigned to actual pins in file "pines.pcf")
module main(
	input  CLKIN, 		// Input clock from crystal oscillator (16MHz)
	
	// SPI
	output LORA_SCK, ICE_SCK,
	output LORA_MOSI, ICE_MOSI,
	input  LORA_MISO, ICE_MISO,	
	output LORA_CS, ADC_CS,	

	//GPO
	output EN_5V_UP, EN_5V_M4, EN_1V4_M4, M2_ON_OFF,
	output L_RX, L_TX,
	output ICE_LED1, ICE_LED2, ICE_LED3, ICE_LED4,
	output LORA_RESET,
	
	//GPI
	input LORA_DIO1,
	input LORA_BUSY, 
	
	// UART	
	input RXD,
	output TXD,
	input RXD1,
	output TXD1,
	input RXD2,
	output TXD2,

	output BME680_CS
);

//-- PLL: generates a 25MHz master clock from a 16MHz input
wire clk,pll_lock;

pll
  pll1(
	.clock_in(CLKIN),
	.clock_out(clk),
	.locked(pll_lock)
	);

wire [31:0] gpout_bus;
wire [31:0] gpin_bus;
assign { LORA_RESET, L_RX, L_TX, EN_5V_UP, EN_5V_M4, EN_1V4_M4, M2_ON_OFF,
         ICE_LED3, ICE_LED2, ICE_LED1, ICE_LED4 } = gpout_bus[10:0];
assign gpin_bus[1:0] = { LORA_DIO1, LORA_BUSY };


// Instance of the system
SYSTEM sys1(
    .clk   (clk),
    .reset (reset),

    .rxd   (RXD),
    .txd   (TXD),

    .rxd1  (RXD1),
    .txd1  (TXD1),

    .rxd2  (RXD2),
    .txd2  (TXD2),

    .lora_sck   (LORA_SCK),
    .lora_mosi  (LORA_MOSI),
    .lora_miso  (LORA_MISO),
    .lora_cs    (LORA_CS),

    .ice_sck  (ICE_SCK),
    .ice_mosi (ICE_MOSI),
    .ice_miso (ICE_MISO),
    .adc_cs   (ADC_CS), 
    .bme680_cs (BME680_CS),
    

    .salida (gpout_bus),
    .entrada  (gpin_bus)


);


// Automatic RESET pulse: Reset is held active for 255 cycles after PLL lock

reg [21:0]cnt=22'h3fffff;
wire reset=(cnt!=0);

always @(posedge clk) cnt<= reset ? cnt-1: cnt;


endmodule


