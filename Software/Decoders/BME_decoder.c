void BME_write(uint8_t addr, uint8_t dato)  
{ 
	SPISS  = 0b01;         // BME_ADC = 0 
	SPIDAT = addr & 0x7F;  // clears most significant bit
	while(SPISTA & 1);     // Esperamos a que se complete la transferencia 
	SPIDAT = dato;         // Ponemos el dato 
	while(SPISTA & 1);     // Esperamos a que se complete la transferencia 
	SPISS  = 0b11;         // BME_ADC = 1 
} 

uint8_t BME_read(uint8_t addr)  
{ 
	uint8_t d;    
	if(addr&0x80) BME_write(0x73, 0x00); // Página 0, 0x80..0xFF 
	   
	SPISS  = 0b01;          // BME_ADC = 0 
	SPIDAT = (1<<7) | addr; // RD (bit 7) = 1 y dirección (bits 6..0) 
	while(SPISTA & 1);      // Esperamos a que se complete la transferencia 
	SPIDAT = 0 ;            // Byte dummy, para recoger lectura 
	while(SPISTA & 1);      // Esperamos a que se complete la transferencia 
	d = SPIDAT; 
	SPISS  = 0b11;          // BME_ADC = 1 
	 
	BME_write(0x73, (1<<4));  // Página 1, 0x00..0x7F 
	return d; 
} 

uint8_t BME_temp()
{
	//temp_adc 0x24<7:4>/ 0x23 / 0x22  (LSB//MSB);
	temp_adc =BME_read(0x24);
	par_t1 = (BME_read(0xEA)<<8)|BME_read(0xE9);
	var1=((int32_t)temp_adc>>3) - ((int32_t)par_t1<<1);
}


void BME_quick_start()
{
	//QUICK START
	BME_write(0x73,0b00010000);

	//Humidity oversampling 1x
	BME_write(0x72,0b00000001);
	
	//Temperature2x pressure16x
	BME_write(0x74,0b01010100);
	
	//Gas wait 100ms
	BME_write(0x6D,0x59);
	
	//Heater set-point
	BME_write(0x63,0b0);
	
	//nb.conv=0 run_gas_l=1
	BME_write(0x71,0b00010000);
	
	//mode 1
	BME_write(0x74,0b01010101);
}

int main(void){
	BME_quick_start();
	
}