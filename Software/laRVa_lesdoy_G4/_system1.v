//======================================================================
//  SYSTEM.v
//======================================================================

`include "laRVa.v"
`include "uart.v"

module SYSTEM (
    input clk,
    input reset,

    input  rxd,   output txd,   // UART0
    input  rxd1,  output txd1,  // UART1
    input  rxd2,  output txd2,  // UART2

    output sck,   output mosi,  input miso,   // SPI0
    output sck1,  output mosi1, input miso1,  // SPI1

    output [31:0] gpout,
    input  [31:0] gpin,

	output fss
);

wire cclk = clk;

//======================================================================
// CPU
//======================================================================
wire [31:0] ca;
wire [31:0] cdo;
wire [3:0]  mwe;
wire [31:0] cdi;
wire        irq;
wire [31:2] ivector;
wire        trap;

laRVa cpu (
    .clk     (cclk),
    .reset   (reset),
    .addr    (ca[31:2]),
    .wdata   (cdo),
    .wstrb   (mwe),
    .rdata   (cdi),
    .irq     (irq),
    .ivector (ivector),
    .trap    (trap)
);

//======================================================================
// Memory map: RAM interna / IO
//======================================================================
wire iramcs = (ca[31:29]==3'b000);
wire iocs   = (ca[31:29]==3'b111);

reg [31:0] cdi_r;
assign cdi = cdi_r;

wire [31:0] mdo;
wire [31:0] iodo;

always @* begin
    casex ({iocs, iramcs})
        2'b01: cdi_r = mdo;
        2'b10: cdi_r = iodo;
        default: cdi_r = 32'hxxxxxxxx;
    endcase
end

//---------------- RAM interna 16KB ----------------
ram32 ram0 (
    .clk       (~cclk),
    .re        (iramcs),
    .wrlanes   (iramcs ? mwe : 4'b0000),
    .addr      (ca[13:2]),     // 4096 palabras * 4 bytes = 16KB
    .data_read (mdo),
    .data_write(cdo)
);

//======================================================================
//  PERIFÉRICOS: Chip selects / constantes
//======================================================================

// UART0: 0xE0000080, UART1: 0xE0000090, UART2: 0xE00000A0
wire uartcs0 = iocs & (ca[7:4]==4'b1000);
wire uartcs1 = iocs & (ca[7:4]==4'b1001);
wire uartcs2 = iocs & (ca[7:4]==4'b1010);

// SPI1: 0xE0000060, SPI0: 0xE0000070
wire spi1cs  = iocs & (ca[7:4]==4'b0110);
wire spi0cs  = iocs & (ca[7:4]==4'b0111);

// GPOUT/GPIN: 0xE0000030 / 0xE0000034
localparam GPOUT_WORD = 6'b001100;
localparam GPIN_WORD  = 6'b001101;

// TIMER: 0xE0000040
localparam TIMER_WORD = 6'b010000;

// IRQ_ENABLE: 0xE0000020
localparam IRQEN_WORD = 6'b001000;

//======================================================================
//  UART0 / UART1 / UART2
//======================================================================

//------------- UART0 -------------
wire tend0, thre0, dv0, fe0, ove0;
wire [7:0] uart0_do;
wire uwrtx0   = uartcs0 & (~ca[2]) & mwe[0];
wire uwrbaud0 = uartcs0 & ( ca[2]) & mwe[0] & mwe[1];
wire urd0     = uartcs0 & (~ca[2]) & (mwe==4'b0000);

UART_CORE #(.BAUDBITS(12)) uart0 (
    .clk    (cclk),
    .txd    (txd),
    .rxd    (rxd),
    .d      (cdo[15:0]),
    .wrtx   (uwrtx0),
    .wrbaud (uwrbaud0),
    .rd     (urd0),
    .q      (uart0_do),
    .dv     (dv0),
    .fe     (fe0),
    .ove    (ove0),
    .tend   (tend0),
    .thre   (thre0)
);

//------------- UART1 -------------
wire tend1, thre1, dv1, fe1, ove1;
wire [7:0] uart1_do;
wire uwrtx1   = uartcs1 & (~ca[2]) & mwe[0];
wire uwrbaud1 = uartcs1 & ( ca[2]) & mwe[0] & mwe[1];
wire urd1     = uartcs1 & (~ca[2]) & (mwe==4'b0000);

UART_CORE #(.BAUDBITS(12)) uart1 (
    .clk    (cclk),
    .txd    (txd1),
    .rxd    (rxd1),
    .d      (cdo[15:0]),
    .wrtx   (uwrtx1),
    .wrbaud (uwrbaud1),
    .rd     (urd1),
    .q      (uart1_do),
    .dv     (dv1),
    .fe     (fe1),
    .ove    (ove1),
    .tend   (tend1),
    .thre   (thre1)
);

//------------- UART2 -------------
wire tend2, thre2, dv2, fe2, ove2;
wire [7:0] uart2_do;
wire uwrtx2   = uartcs2 & (~ca[2]) & mwe[0];
wire uwrbaud2 = uartcs2 & ( ca[2]) & mwe[0] & mwe[1];
wire urd2     = uartcs2 & (~ca[2]) & (mwe==4'b0000);

UART_CORE #(.BAUDBITS(12)) uart2 (
    .clk    (cclk),
    .txd    (txd2),
    .rxd    (rxd2),
    .d      (cdo[15:0]),
    .wrtx   (uwrtx2),
    .wrbaud (uwrbaud2),
    .rd     (urd2),
    .q      (uart2_do),
    .dv     (dv2),
    .fe     (fe2),
    .ove    (ove2),
    .tend   (tend2),
    .thre   (thre2)
);

//======================================================================
//  TIMER (MAX_COUNT / TIMER + IRQ)
//======================================================================

reg [31:0] max_count      = 32'h0000FFFF;
reg [31:0] timer          = 32'h00000000;
reg        timer_irq_flag = 1'b0;

wire timer_we = iocs & (ca[7:2]==TIMER_WORD) & |mwe;         // write MAX_COUNT
wire timer_re = iocs & (ca[7:2]==TIMER_WORD) & (mwe==4'b0);  // read TIMER

always @(posedge cclk or posedge reset) begin
    if (reset) begin
        max_count      <= 32'h0000FFFF;
        timer          <= 32'h00000000;
        timer_irq_flag <= 1'b0;
    end else if (timer_we) begin
        max_count      <= cdo;
        timer          <= 0;
        timer_irq_flag <= 1'b0;
    end else begin
        if (timer >= max_count) begin
            timer          <= 0;
            timer_irq_flag <= 1'b1;
        end else begin
            timer          <= timer + 1;
        end
        if (timer_re)
            timer_irq_flag <= 1'b0;
    end
end

wire irq_timer = timer_irq_flag;

//======================================================================
//  GPIN / GPOUT
//======================================================================

reg [31:0] gpout_reg = 32'h0000_0000;
assign gpout = gpout_reg;

wire gpout_we = iocs & (ca[7:2]==GPOUT_WORD) & |mwe;

always @(posedge cclk or posedge reset) begin
    if (reset)
        gpout_reg <= 32'h0000_0000;
    else if (gpout_we)
        gpout_reg <= cdo;
end

//======================================================================
//  SPI0 / SPI1 (FSM modo 0, 8 bits)
//======================================================================

//---------------- SPI0: 0xE0000070 (DAT), 0xE0000074 (CTL/FLAGS), 0xE0000078 (SS)
wire spi0_dat_sel = iocs & (ca[7:2]==6'b011100); // 0x70
wire spi0_ctl_sel = iocs & (ca[7:2]==6'b011101); // 0x74
wire spi0_ss_sel  = iocs & (ca[7:2]==6'b011110); // 0x78

wire spi0_dat_we  = spi0_dat_sel & |mwe;
wire spi0_ctl_we  = spi0_ctl_sel & |mwe;
wire spi0_ss_we   = spi0_ss_sel  & |mwe;

reg [31:0] spi0_txdata   = 32'h0;
reg [31:0] spi0_control  = 32'h0000_0007; // DIVIDER[7:0], DLEN ignorado aquí
reg [1:0]  spi0_ss       = 2'b11;
reg        spi0_busy     = 1'b0;

reg [7:0]  spi0_shift_tx = 8'h00;
reg [7:0]  spi0_shift_rx = 8'h00;
reg [3:0]  spi0_bitcnt   = 4'd0;

reg [7:0]  spi0_divcnt   = 8'd0;
reg        spi0_sck_reg  = 1'b0;
reg        spi0_sck_en   = 1'b0;

assign sck  = spi0_sck_en ? spi0_sck_reg : 1'b0;
assign mosi = spi0_sck_en ? spi0_shift_tx[7] : 1'b0;

wire [7:0] spi0_divider = spi0_control[7:0];

always @(posedge cclk or posedge reset) begin
    if (reset) begin
        spi0_txdata   <= 32'h0;
        spi0_control  <= 32'h0000_0007;
        spi0_ss       <= 2'b11;
        spi0_busy     <= 1'b0;
        spi0_shift_tx <= 8'h00;
        spi0_shift_rx <= 8'h00;
        spi0_bitcnt   <= 4'd0;
        spi0_divcnt   <= 8'd0;
        spi0_sck_reg  <= 1'b0;
        spi0_sck_en   <= 1'b0;
    end else begin
        if (spi0_ctl_we)
            spi0_control <= cdo;
        if (spi0_ss_we)
            spi0_ss <= cdo[1:0];

        if (spi0_dat_we && !spi0_busy) begin
            spi0_txdata   <= cdo;
            spi0_shift_tx <= cdo[7:0];
            spi0_shift_rx <= 8'h00;
            spi0_bitcnt   <= 4'd8;
            spi0_busy     <= 1'b1;
            spi0_sck_en   <= 1'b1;
            spi0_sck_reg  <= 1'b0;
            spi0_divcnt   <= 8'd0;
        end else if (spi0_busy) begin
            if (spi0_divcnt >= spi0_divider) begin
                spi0_divcnt  <= 8'd0;
                spi0_sck_reg <= ~spi0_sck_reg;

                if (spi0_sck_reg == 1'b1) begin
                    // flanco alto: muestrea MISO
                    spi0_shift_rx <= {spi0_shift_rx[6:0], miso};
                end else begin
                    // flanco bajo: desplaza MOSI
                    spi0_shift_tx <= {spi0_shift_tx[6:0], 1'b0};
                    if (spi0_bitcnt != 0)
                        spi0_bitcnt <= spi0_bitcnt - 1'b1;
                    if (spi0_bitcnt == 1) begin
                        spi0_busy    <= 1'b0;
                        spi0_sck_en  <= 1'b0;
                        spi0_sck_reg <= 1'b0;
                        spi0_txdata  <= {24'h0, spi0_shift_rx};
                    end
                end
            end else begin
                spi0_divcnt <= spi0_divcnt + 1'b1;
            end
        end
    end
end

wire [31:0] spi0_flags = {31'h0, spi0_busy};

//---------------- SPI1: 0xE0000060 (DAT), 0xE0000064 (CTL/FLAGS), 0xE0000068 (SS)
wire spi1_dat_sel = iocs & (ca[7:2]==6'b011000); // 0x60
wire spi1_ctl_sel = iocs & (ca[7:2]==6'b011001); // 0x64
wire spi1_ss_sel  = iocs & (ca[7:2]==6'b011010); // 0x68

wire spi1_dat_we  = spi1_dat_sel & |mwe;
wire spi1_ctl_we  = spi1_ctl_sel & |mwe;
wire spi1_ss_we   = spi1_ss_sel  & |mwe;

reg [31:0] spi1_txdata   = 32'h0;
reg [31:0] spi1_control  = 32'h0000_0007;
reg [1:0]  spi1_ss       = 2'b11;
reg        spi1_busy     = 1'b0;

reg [7:0]  spi1_shift_tx = 8'h00;
reg [7:0]  spi1_shift_rx = 8'h00;
reg [3:0]  spi1_bitcnt   = 4'd0;

reg [7:0]  spi1_divcnt   = 8'd0;
reg        spi1_sck_reg  = 1'b0;
reg        spi1_sck_en   = 1'b0;

assign sck1  = spi1_sck_en ? spi1_sck_reg : 1'b0;
assign mosi1 = spi1_sck_en ? spi1_shift_tx[7] : 1'b0;

wire [7:0] spi1_divider = spi1_control[7:0];

always @(posedge cclk or posedge reset) begin
    if (reset) begin
        spi1_txdata   <= 32'h0;
        spi1_control  <= 32'h0000_0007;
        spi1_ss       <= 2'b11;
        spi1_busy     <= 1'b0;
        spi1_shift_tx <= 8'h00;
        spi1_shift_rx <= 8'h00;
        spi1_bitcnt   <= 4'd0;
        spi1_divcnt   <= 8'd0;
        spi1_sck_reg  <= 1'b0;
        spi1_sck_en   <= 1'b0;
    end else begin
        if (spi1_ctl_we)
            spi1_control <= cdo;
        if (spi1_ss_we)
            spi1_ss <= cdo[1:0];

        if (spi1_dat_we && !spi1_busy) begin
            spi1_txdata   <= cdo;
            spi1_shift_tx <= cdo[7:0];
            spi1_shift_rx <= 8'h00;
            spi1_bitcnt   <= 4'd8;
            spi1_busy     <= 1'b1;
            spi1_sck_en   <= 1'b1;
            spi1_sck_reg  <= 1'b0;
            spi1_divcnt   <= 8'd0;
        end else if (spi1_busy) begin
            if (spi1_divcnt >= spi1_divider) begin
                spi1_divcnt  <= 8'd0;
                spi1_sck_reg <= ~spi1_sck_reg;

                if (spi1_sck_reg == 1'b1) begin
                    spi1_shift_rx <= {spi1_shift_rx[6:0], miso1};
                end else begin
                    spi1_shift_tx <= {spi1_shift_tx[6:0], 1'b0};
                    if (spi1_bitcnt != 0)
                        spi1_bitcnt <= spi1_bitcnt - 1'b1;
                    if (spi1_bitcnt == 1) begin
                        spi1_busy    <= 1'b0;
                        spi1_sck_en  <= 1'b0;
                        spi1_sck_reg <= 1'b0;
                        spi1_txdata  <= {24'h0, spi1_shift_rx};
                    end
                end
            end else begin
                spi1_divcnt <= spi1_divcnt + 1'b1;
            end
        end
    end
end

wire [31:0] spi1_flags = {31'h0, spi1_busy};

//======================================================================
//  INTERRUPCIONES (8 vectores)
//======================================================================

reg [7:0] irqen = 8'h00;

always @(posedge cclk or posedge reset) begin
    if (reset)
        irqen <= 8'h00;
    else if (iocs & (ca[7:2]==IRQEN_WORD) & |mwe)
        irqen <= cdo[7:0];
end

reg [31:2] irqvect[0:7];

assign v2={irqvect[2],2'b00};

integer k;
always @(posedge cclk or posedge reset) begin
    if (reset) begin
        for (k=0;k<8;k=k+1) irqvect[k] <= 30'h0;
    end else if (iocs & (ca[7:2] < 6'd8) & |mwe) begin
        irqvect[ca[4:2]] <= cdo[31:2];
    end
end

wire [7:1] irqsrc;
assign irqsrc[1] = irq_timer;
assign irqsrc[2] = dv0;
assign irqsrc[3] = thre0;
assign irqsrc[4] = dv1;
assign irqsrc[5] = thre1;
assign irqsrc[6] = dv2;
assign irqsrc[7] = thre2;

wire [7:1] irqpen;
assign irqpen[1] = irqen[1] & irqsrc[1];
assign irqpen[2] = irqen[2] & irqsrc[2];
assign irqpen[3] = irqen[3] & irqsrc[3];
assign irqpen[4] = irqen[4] & irqsrc[4];
assign irqpen[5] = irqen[5] & irqsrc[5];
assign irqpen[6] = irqen[6] & irqsrc[6];
assign irqpen[7] = irqen[7] & irqsrc[7];

reg [2:0] vecn_irq;
always @* begin
    casex (irqpen)
        7'b1xxxxxx: vecn_irq = 3'd1; // TIMER
        7'b01xxxxx: vecn_irq = 3'd2; // RX0
        7'b001xxxx: vecn_irq = 3'd3; // TX0
        7'b0001xxx: vecn_irq = 3'd4; // RX1
        7'b00001xx: vecn_irq = 3'd5; // TX1
        7'b000001x: vecn_irq = 3'd6; // RX2
        7'b0000001: vecn_irq = 3'd7; // TX2
        default:     vecn_irq = 3'd0;
    endcase
end

wire [2:0] vecn = trap ? 3'd0 : vecn_irq;
assign ivector = irqvect[vecn];
assign irq     = trap | (irqpen != 7'b0000000);

//======================================================================
//  MUX lectura periféricos
//======================================================================

reg [31:0] iodo_r;
assign iodo = iodo_r;

always @* begin
    case (ca[7:2])
        // UART0
        6'b1000_00: iodo_r = {24'h0, uart0_do};
        6'b100001: iodo_r = {27'h0, ove0, fe0, tend0, thre0, dv0};
        // UART1
        6'b100100: iodo_r = {24'h0, uart1_do};
        6'b100101: iodo_r = {27'h0, ove1, fe1, tend1, thre1, dv1};
        // UART2
        6'b101000: iodo_r = {24'h0, uart2_do};
        6'b101001: iodo_r = {27'h0, ove2, fe2, tend2, thre2, dv2};
        // TIMER
        6'b010000: iodo_r = timer;
        // SPI1
        6'b011000: iodo_r = spi1_txdata;
        6'b011001: iodo_r = spi1_flags;
        6'b011010: iodo_r = {30'h0, spi1_ss};
        // SPI0
        6'b011100: iodo_r = spi0_txdata;
        6'b011101: iodo_r = spi0_flags;
        6'b011110: iodo_r = {30'h0, spi0_ss};
        // GPOUT / GPIN
        6'b001100: iodo_r = gpout_reg;
        6'b001101: iodo_r = gpin;
        // IRQ_ENABLE
        6'b001000: iodo_r = {24'h0, irqen};
        default: begin
            if (iocs && (ca[7:2] < 6'd8))
                iodo_r = {irqvect[ca[4:2]], 2'b00};
            else
                iodo_r = 32'hxxxxxxxx;
        end
    endcase
end

endmodule

//======================================================================
//  RAM32
//======================================================================
module ram32
(
    input              clk,
    input              re,
    input      [3:0]   wrlanes,
    input      [11:0]  addr,
    output     [31:0]  data_read,
    input      [31:0]  data_write
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
