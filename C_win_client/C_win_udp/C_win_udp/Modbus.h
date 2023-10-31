#pragma once
union Data
{
	char data_t[2];
	unsigned short int data_u;
};

struct Modbus
{
	char address;
	char function;
	union Data crc;
};



struct Modbus_ask : Modbus
{
	union Data offset;
	union Data reg_count;
};
struct Modbus_answer : Modbus
{
	unsigned short int data[255];
};


unsigned short int CRC(char* data, int length, unsigned int crcTable[256]);
void ModbusInit(unsigned int crcTable[256]);