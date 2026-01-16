int MCP3004_Read_Reg(unsigned char canal){
	int temp1, temp2;
	uint32_t resultado = 0;
	//ESTO NO FUNCIONA xd
	SPISS = ADC_CS;
	spixfer(MCP3004_START);
	temp1 = spixfer(canal);
	temp2 = spixfer(0x00);
	SPISS = 0b11;
	resultado = temp1 << 8;
	resultado |= temp2;
	return (resultado &= 0b0000001111111111);
}
