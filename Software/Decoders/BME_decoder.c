union partX{
	uint32_t combined;
	struct{
		uint8_t bytelow; 
		uint8_t bytehigh=0x00;
		uint16_t padding=0x0000;
	}parts;
};

union part_adc{
	uint32_t combined;
	struct{
		uint8_t bytelow;
		uint8_t byteinter;
		uint8_t bytehigh=0x00;
		uint8_t padding=0x00;
	}parts;
};

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


uint32_t BME_gettemperature(bool return_tfine)
{
	//temp_adc 0x24<7:4>/ 0x23 / 0x22  (LSB//MSB);


	union part_adc temp_adc;
	union partX par_t1;
	union partX par_t2;
	union partX par_t3;
	
	
	temp_adc.parts.bytelow=BME_read(0x24)>>4;
	temp_adc.parts.byteinter=BME_read(0x23);
	temp_adc.parts.bytehigh=BME_read(0x22);
	
	par_t1.parts.bytelow=BME_read(0xE9);
	par_t1.parts.bytehigh=BME_read(0xEA);
	
	par_t2.parts.bytelow=BME_read(0x8A);
	par_t2.parts.bytehigh=BME_read(0x8B);
	
	par_t3.parts.bytelow=BME_read(0x8C);	
	

	uint32_t var1=((int32_t)temp_adc.combined>>3) - ((int32_t)par_t1.combined<<1);
	uint32_t var2=(var1 * par_t2.combined)>>11;
	uint32_t var3=((((var1>>1) * (var1>>1))>>12)*(par_t3.combined<<4))>>14;
	if(return_tfine)return var2+var3;
	else return ((var2+var3) * 5+128)>>8;
}


uint32_t BME_getpressure(){
	union partX par_p1;
	union partX par_p2;
	union partX par_p3;
	union partX par_p4;
	union partX par_p5;
	union partX par_p6;
	union partX par_p7;
	union partX par_p8;
	union partX par_p9;
	union partX par_p10;
	union part_adc press_adc;
	
	par_p1.parts.bytelow=BME_read(0x8E);
	par_p1.parts.bytehigh=BME_read(0x8F);
	par_p2.parts.bytelow=BME_read(0x90);
	par_p2.parts.bytehigh=BME_read(0x91);
	par_p3.parts.bytelow=BME_read(0x92);
	par_p4.parts.bytelow=BME_read(0x94);
	par_p4.parts.bytehigh=BME_read(0x95);
	par_p5.parts.bytelow=BME_read(0x96);
	par_p5.parts.bytehigh=BME_read(0x97);
	par_p6.parts.bytelow=BME_read(0x99);
	par_p7.parts.bytelow=BME_read(0x98);
	par_p8.parts.bytelow=BME_read(0x9C);
	par_p8.parts.bytehigh=BME_read(0x9D);
	par_p9.parts.bytelow=BME_read(0x9E);
	par_p9.parts.bytehigh=BME_read(0x9F);
	par_p10.parts.bytelow=BME_read(0xA0);
	press_adc.parts.bytelow=BME_read(0x21)>>4;
	press_adc.parts.byteinter=BME_read(0x20);
	press_adc.parts.bytehigh=BME_read(0x1F);
	
	uint32_t var1=(BME_gettemperature(true)>>1)-64000;
	uint32_t var2=((((var1>>2)*(var1>>2))>>11) * par_p6.combined)>>2;
	var2=var2+((var1 * par_p5.combined)<<1);
	var2=(var2>>2)+(par_p4.combined<<16);
	var1=(((var1>>2)*(var1>>2))>>13)*((par_p3.combined<<5)>>3)+((par_p2.combined * var1)>>1);
	var1=var1>>18;
	var1=((32768+var1) * par_p1.combined)>>15;
	press_comp=1048576-press_adc.combined;
	press_comp=(press_comp-(var2>>12))*(uint32_t)3125;
	press_comp=press_comp/var1;
	if(press_comp>=(1<<30)) press_comp=press_comp<<1;
	var1=(par_p9.combined * ((press_comp>>3)*(press_comp>>3))>>13)>>12;
	var2=((press_comp>>2) * par_p8.combined)>>13;
	var3=((press_comp>>8)*(press_comp>>8)*(press_comp>>8) * par_p10.combined)>>17;
	
	return press_comp+((var1+var2+var3+(par_p7.combined<<7))>>4);
}

uint32_t BME_gethumidity(){
	uint32_t temp_scaled=BME_gettemperature(false);
	union partX par_h1;
	union partX par_h2;
	union partX par_h3;
	union partX par_h4;
	union partX par_h5;
	union partX par_h6;
	union partX par_h7;
	union part_adc hum_adc;
	
	par_h1.parts.bytelow=(BME_read(0xE2)<<4)>>4;
	par_h1.parts.bytehigh=BME_read(0x8F);
	par_h2.parts.bytelow=BME_read(0xE4);
	par_h2.parts.bytehigh=BME_read(0xE5);
	par_h3.parts.bytelow=BME_read(0xE4);
	par_h4.parts.bytelow=BME_read(0xE5);
	par_h5.parts.bytelow=BME_read(0xE6);
	par_h6.parts.bytelow=BME_read(0xE7);
	par_h7.parts.bytelow=BME_read(0xE8);
	hum_adc.parts.bytelow=BME_read(0x26);
	hum_adc.parts.byteinter=BME_read(0x25);
	hum_adc.parts.bytehigh=BME_read(0x00);
	
	uint32_t var1=hum_adc.combine - (par_h1<<4) - (temp_scaled * par_h3.combined /((uint32_t)100) >>1);
	uint32_t var2=(par_h2.combined * (temp_scaled * par_h4.combined /((uint32_t)100) + ((temp_scaled * temp_scaled * par_h5.combined /(uint32_t)100)>>6) /((uint32_t)100) + ((uint32_t)(1<<14)))) >>10;
	uint32_t var3=var1*var2;
	uint32_t var4=((par_h6.combined<<7) + temp_scaled * par_h7.combined /((uint32_t)100)) >>4;
	uint32_t var6 = (var4 * ((var3>>14)*(var3 >>14))>>10)>>1;
	return (((var3+var6)>>10)*((uint32_t) 1000)) >>12;
	
}
uint8_t BME_heat_gas(uint32_t target_temp)
{
	union partX par_g1;
	union partX par_g2;
	union partX par_g3;
	union partX res_heat_range;
	union partX res_heat_val;
	
	par_g1.parts.bytelow=BME_read(0xED);
	par_g2.parts.bytelow=BME_read(0xEB);
	par_g2.parts.bytehigh=BME_read(0xEC);
	par_g3.parts.bytelow=BME_read(0xEE);
	res_heat_range.parts.bytelow=(BME_read(0x02)<<2)>>6; //Necesita multiplicación bit a bit para sacar los bits 5:4;
	res_heat_val.parts.bytelow=0x00;
	
	uint32_t var1= (BME_gettemperature(false) * par_g3.combined /10)<<8;
	uint32_t var2 = (par_g1.combined + (uin32_t)784) * (((((par_g2.combined + 154009) * target_temp * 5)/100)+3276800)/10);
	uint32_t var4 = (var1 + (var2 >>1)) / (res_heat_range +4);
	uint32_t var5 = (131* res_heat_val) + 65536;
	uint32_t res_heat_x100 =(var4/var5 -250) * 34;
	uint32_t res_heat_x=(uint8_t)((res_heat_x100 +50)/100);
}

uint32_t BME_get_gas_res()
{
	union partX gas_adc;
	union partX gas_range;
	union partX range_switching_error;
	
	gas_adc.parts.bytelow=BME_read(0x2B)>>6;
	gas_adc.parts.bytehigh=BME_read(0x2A);
	gas_range.parts.bytelow=(BME_read(0x2B)<<4)>>4;
	range_switching_error.bytelow=BME_read(0x04);
	
	uint64_t var1 =(uint64_t)(((1340+5*(uint64_t)range_switching_error) * (uint64_t)const_array1_int[gas_range])>>16);
	
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