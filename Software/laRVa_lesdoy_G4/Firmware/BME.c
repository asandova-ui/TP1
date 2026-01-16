void startBME680(){
	writeBME680(0xB6,RESET_BME);
	writeBME680(0x10,STATUS_BME);
	writeBME680(0x01,CTRL_HUM);
	writeBME680(0x54,CTRL_MEAS);
	writeBME680(0x59,gas_wait0);
	writeBME680(0x00,res_heat0);
	writeBME680(0x10,CTRL_GAS_1);
	writeBME680(0x55,CTRL_MEAS);
}

void writeBME680(char dato,char direccion){
	SPISS = CS_BME680;
	spixfer(0x00|direccion);
	spixfer(dato);
	SPISS = 0b11;
}

char readBME680(char direccion){
	unsigned char resultado;
	SPISS = CS_BME680;
	spixfer(0x80|direccion);
	resultado = spixfer(0);
	SPISS=0b11;
	return resultado;
}

int returnTemp(){
	int coef_t1, coef_t2, coef_t3, adc_temp, calc1, calc2, calc3, fine_temp, temp_calc;
	writeBME680(0x00,STATUS_BME);
	coef_t1 = (int32_t)((readBME680(0xEA) << 8) | readBME680(0xE9));
	coef_t2 = (int32_t)((readBME680(0x8B) << 8) | readBME680(0x8A));
	coef_t3 = (int32_t)readBME680(0x8C);
	writeBME680(0x10,STATUS_BME);
	adc_temp = (int32_t)((readBME680(0x22) << 12) | (readBME680(0x23) << 4) | (readBME680(0x24) >> 4));
	calc1 = ((int32_t)adc_temp >> 3) - ((int32_t)coef_t1 << 1);
	calc2 = (calc1 * (int32_t)coef_t2) >> 11;
	calc3 = ((((calc1 >> 1) * (calc1 >> 1)) >> 12) * ((int32_t)coef_t3 << 4)) >> 14;
	fine_temp = calc2 + calc3;
	temp_calc = ((fine_temp * 5) + 128) >> 8;
	return temp_calc;
}

int returnPressure(){
	int coef_p1, coef_p2, coef_p3, coef_p4, coef_p5, coef_p6, coef_p7, coef_p8, coef_p9, coef_p10, adc_press, press_calc;
	int calc1, calc2, calc3, fine_temp;
	writeBME680(0x00,STATUS_BME);
	coef_p1 = (int32_t)((readBME680(0x8F) << 8) | readBME680(0x8E));
	coef_p2 = (int32_t)((readBME680(0x91) << 8) | readBME680(0x90));
	coef_p3 = (int32_t)readBME680(0x92);
	coef_p4 = (int32_t)((readBME680(0x95) << 8) | readBME680(0x94));
	coef_p5 = (int32_t)((readBME680(0x97) << 8) | readBME680(0x96));
	coef_p6 = (int32_t)readBME680(0x99);
	coef_p7 = (int32_t)readBME680(0x98);
	coef_p8 = (int32_t)((readBME680(0x9D) << 8) | readBME680(0x9C));
	coef_p9 = (int32_t)((readBME680(0x9F) << 8) | readBME680(0x9E));
	coef_p10 = (int32_t)readBME680(0xA0);
	writeBME680(0x10,STATUS_BME);
	adc_press = (int32_t)((readBME680(0x1F) << 12) | (readBME680(0x20) << 4) | (readBME680(0x21) >> 4));
	calc1 = ((int32_t)fine_temp >> 1) - 64000;
	calc2 = ((((calc1 >> 2) * (calc1 >> 2)) >> 11) * (int32_t)coef_p6) >> 2;
	calc2 = calc2 + ((calc1 * (int32_t)coef_p5) << 1);
	calc2 = (calc2 >> 2) + ((int32_t)coef_p4 << 16);
	calc1 = (((((calc1 >> 2) * (calc1 >> 2)) >> 13) * ((int32_t)coef_p3 << 5)) >> 3) + (((int32_t)coef_p2 * calc1) >> 1);
	calc1 = calc1 >> 18;
	calc1 = ((32768 + calc1) * (int32_t)coef_p1) >> 15;
	press_calc = 1048576 - adc_press;
	press_calc = (uint32_t)((press_calc - (calc2 >> 12)) * ((uint32_t)3125));
	if (press_calc >= (1 << 30)) press_calc = ((press_calc / (uint32_t)calc1) << 1);
	else press_calc = ((press_calc << 1) / (uint32_t)calc1);
	calc1 = ((int32_t)coef_p9 * (int32_t)(((press_calc >> 3) * (press_calc >> 3)) >> 13)) >> 12;
	calc2 = ((int32_t)(press_calc >> 2) * (int32_t)coef_p8) >> 13;
	calc3 = ((int32_t)(press_calc >> 8) * (int32_t)(press_calc >> 8) * (int32_t)(press_calc >> 8) * (int32_t)coef_p10) >> 17;
	press_calc = (int32_t)(press_calc) + ((calc1 + calc2 + calc3 + ((int32_t)coef_p7 << 7)) >> 4);
	return press_calc;
}

int returnHUMIDITY(int temp_calc){
	int coef_h1, coef_h2, coef_h3, coef_h4, coef_h5, coef_h6, coef_h7, adc_hum, temp_escalado, hum_calc;
	int calc1, calc2, calc3, calc4, calc5, calc6;
	writeBME680(0x00,STATUS_BME);
	coef_h1 = (int32_t)((readBME680(0xE3) << 4) | (readBME680(0xE2) & 0b00001111));
	coef_h2 = (int32_t)((readBME680(0xE1) << 4) | (readBME680(0xE2) >> 4));
	coef_h3 = (int32_t)readBME680(0xE4);
	coef_h4 = (int32_t)readBME680(0xE5);
	coef_h5 = (int32_t)readBME680(0xE6);
	coef_h3 = (int32_t)readBME680(0xE7);
	coef_h3 = (int32_t)readBME680(0xE8);
	writeBME680(0x10,STATUS_BME);
	adc_hum = (int32_t)((readBME680(0x25) << 8) | readBME680(0x26));
	temp_escalado = (int32_t)temp_calc;
	calc1 = (int32_t)adc_hum - (int32_t)((int32_t)coef_h1 << 4) - (((temp_escalado * (int32_t)coef_h3) / ((int32_t)100)) >> 1);
	calc2 = ((int32_t)coef_h2 * (((temp_escalado * (int32_t)coef_h4) / ((int32_t)100)) + (((temp_escalado * ((temp_escalado * (int32_t)coef_h5) / ((int32_t)100))) >> 6) / ((int32_t)100)) + ((int32_t)(1 << 14)))) >> 10;
	calc3 = calc1 * calc2;
	calc4 = (((int32_t)coef_h6 << 7) + ((temp_escalado * (int32_t)coef_h7) / ((int32_t)100))) >> 4;
	calc5 = ((calc3 >> 14) * (calc3 >> 14)) >> 10;
	calc6 = (calc4 * calc5) >> 1;
	hum_calc = (((calc3 + calc6) >> 10) * ((int32_t) 1000)) >> 12;
	return hum_calc;
}
