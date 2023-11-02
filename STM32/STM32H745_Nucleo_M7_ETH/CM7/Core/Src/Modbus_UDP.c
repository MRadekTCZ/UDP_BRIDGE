//#include "ModbusUDP.h"
unsigned short int CRC_check(char* data, int length, unsigned int crcTable[256])
{
    unsigned int crc_16 = 0xFFFF;
    for (int i = 0; i < length; i++) {
    	crc_16 = (crc_16 << 8) ^ crcTable[((crc_16 >> 8) ^ data[i]) & 0xFF];
    }
    return crc_16;
}

void ModbusInit(unsigned int Table[256]) {
    const unsigned int polynomial = 0x1001; // CRC-16 polinom A001

    for (int i = 0; i < 256; i++) {
        unsigned int crc_16 = i;
        for (int j = 0; j < 8; j++) {
            if (crc_16 & 1) {
            	crc_16 = (crc_16 >> 1) ^ polynomial;
            }
            else {
            	crc_16 >>= 1;
            }
        }
        Table[i] = crc_16;
    }
}
