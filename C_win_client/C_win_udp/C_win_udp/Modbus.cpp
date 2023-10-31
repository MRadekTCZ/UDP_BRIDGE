//#include "Modbus.h"
unsigned short int CRC(char* data, int length, unsigned int crcTable[256])
{
    unsigned int crc = 0xFFFF;
    for (int i = 0; i < length; i++) {
        crc = (crc << 8) ^ crcTable[((crc >> 8) ^ data[i]) & 0xFF];
    }
    return crc;
}

void ModbusInit(unsigned int Table[256]) {
    const unsigned int polynomial = 0x1001; // CRC-16 polinom A001

    for (int i = 0; i < 256; i++) {
        unsigned int crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            }
            else {
                crc >>= 1;
            }
        }
        Table[i] = crc;
    }
}