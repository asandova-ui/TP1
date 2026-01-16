#define FREQ_SPI 1000000
#define DIV_SPI ((CCLK/(2*FREQ_SPI))-1)

unsigned char spixfer(unsigned char dato)
{
	SPIDAT = dato;
	while(SPISTA & 1);
	return SPIDAT;
}

unsigned char spixfer1(unsigned char dato)
{
	SPI1DAT = dato;
	while(SPI1STA & 1);
	return SPI1DAT;
}
