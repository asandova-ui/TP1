void Lectura_Gas(){
	estado = 0;
	ENABLE_5V;
	IRQEN = 0b00000100;
	TIMER = 1 * 1000000;
}

void Lectura_C0(){
	int gas_CO = 1;
	gas_CO = MCP3004_Read_Reg(MCP3004_CH0);
	DISABLE_5V_1V4;
	ENABLE_1V4;
	IRQEN = 0b00000100;
	TIMER = 1 * 1000000;
	estado = 1;
	lectura_CO = (10*(330*gas_CO)) / (1024*7);
	_printf("Medicion de C0 en curso, por favor espere 60 seg.\n");
	_printf("Lectura CO: %d ppm \n", lectura_CO);
	_printf("Medicion de CH4 en curso, por favor espere 90 seg.\n");
}

void Lectura_CH4(){
	int gas_CH4 = 1;
	gas_CH4 = MCP3004_Read_Reg(MCP3004_CH0);
	DISABLE_5V_1V4;
	estado = 2;
	lectura_CH4 = 10*(330*gas_CH4) / (1024);
	_printf("Lectura CH4: %d ppm \n", lectura_CH4);
}
