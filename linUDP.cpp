/////////////
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>
#define CLIENT_PORT 1777
#define SERVER_PORT 777
#define SERVER_IP "192.168.100.201"


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
    char reg_count;
};
struct Modbus_answer : Modbus
{
    char data_count;
    union Data data[255];
};


unsigned short int CRC(char* data, int length, unsigned int crcTable[256]);
void ModbusInit(unsigned int crcTable[256]);
unsigned short int CRC_vector(const char* data, int length, unsigned int crcTable[256]);

unsigned int CRCTable[256] = {
0x0000,0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
0xc601,0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
0xcc01,0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
0x0a00,0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
0xd801,0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
0x1e00,0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
0x1400,0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
0xd201,0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
0xf001,0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
0x3600,0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
0x3c00,0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
0xfa01,0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
0x2800,0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
0xee01,0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
0xe401,0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
0x2200,0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
0xa001,0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
0x6600,0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
0x6c00,0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
0xaa01,0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
0x7800,0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
0xbe01,0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
0xb401,0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
0x7200,0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
0x5000,0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
0x9601,0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
0x9c01,0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
0x5a00,0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
0x8801,0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
0x4e00,0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
0x4400,0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
0x8201,0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

//UDP modbus client template for linux

// Another thread for receiving data via udp
void ReceiveThread(int clientSocket) {
    Modbus_answer response1{};
    char buffer[1024];
    sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);

    while (true) {
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddr, &addrLen);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            int data_inkrement = 0;
            //Writing exact bytes to modbus struct
            response1.address = buffer[0];
            response1.function = buffer[1];
            response1.data_count = buffer[2];
            if (response1.data_count != 0)
            {
                //for loop depends on amount of data incoming
                for (data_inkrement = 0; data_inkrement < response1.data_count; data_inkrement++)
                {
                    response1.data[data_inkrement].data_t[1] = buffer[data_inkrement * 2 + 3];
                    response1.data[data_inkrement].data_t[0] = buffer[data_inkrement * 2 + 4];
                }
                //response1.crc.data_u = CRC_vector(data_count_ptr, length_crc_res, CRCTable);            
                response1.crc.data_t[0] = buffer[data_inkrement * 2 + 3];
                response1.crc.data_t[1] = buffer[data_inkrement * 2 + 4];

                //New array for independent checking CRC sum
                std::vector<char> data_heap_crc_res(response1.data_count * 2 + 3);
                data_heap_crc_res[0] = response1.address;
                data_heap_crc_res[1] = response1.function;
                data_heap_crc_res[2] = response1.data_count;
                int reg_res;
                for (reg_res = 0; reg_res < response1.data_count * 2; reg_res++)
                {
                    data_heap_crc_res[reg_res + 3] = buffer[reg_res + 3];
                }
                const char* data_count_ptr = data_heap_crc_res.data();
                int length_crc_res = sizeof(response1.data_count * 2 + 3);

                //Comparison of received  and counted CRC
                unsigned short int crc_response_comparison = CRC_vector(data_heap_crc_res.data(), length_crc_res, CRCTable);
                bool b_crc_response_comparison = (crc_response_comparison == response1.crc.data_u) ? true : false;

                b_crc_response_comparison = true;
                if (b_crc_response_comparison)
                {
                    for (data_inkrement = 0; data_inkrement < response1.data_count; data_inkrement++)
                    {
                        std::cout << std::dec;
                        // If response for reg setting 
                        if (response1.function == 0x06)
                        {
                            std::cout << "Data set in reg " << response1.data_count << " : " << response1.data[data_inkrement].data_u << std::endl;
                        }
                        //If response for reg asking
                        else { std::cout << "Response " << data_inkrement + 1 << " : " << response1.data[data_inkrement].data_u << std::endl; }

                    }
                    std::cout << std::hex;
                    std::cout << "CRC: " << response1.crc.data_u << std::endl;
                    //std::cout << "CRC: " << crc_response_comparison << std::endl;

                }
                else
                {
                    std::cout << "CRC Error" << "\n";
                }
            }
            else
            {
                std::cout << "Data Error" << "\n";
            }

        }
        else {
            std::cerr << "Error receiving data: " << std::endl;
            break;
        }
    }
}

int main() {

    Modbus_ask Ask1{};
    Ask1.address = 0x01;
    Ask1.function = 0x03;
    Ask1.offset.data_u = 0x000A;
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(CLIENT_PORT);
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    // Binding socket with port number
    if (bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == -1) {
        std::cerr << "Port error." << std::endl;
        close(clientSocket);
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "IP error" << std::endl;
        close(clientSocket);
        return 1;
    }

    // Thread with receving data from server - start
    std::thread receiveThread(ReceiveThread, clientSocket);

    while (true) {

        //std::cout << "Amount of register to read";
        unsigned short int reg_count_read;
        reg_count_read = 1;
        unsigned short int function;
        std::cout << "Reading(3) or setting registers(6)?:";
        std::cin >> function;
        Ask1.function = function;
        unsigned short int reg_cin;
        std::string message;
        if (Ask1.function == 3)
        {
            std::cout << "Number of register to read:";
            std::cin >> reg_count_read;
            Ask1.reg_count = reg_count_read;
            std::cout << "Register offset reading start:";
            std::cin >> message;
            // get register offset          
            reg_cin = std::stoi(message);
            Ask1.offset.data_t[1] = 0xFF;
            Ask1.offset.data_t[0] = reg_cin;
        }
        else if (Ask1.function == 6)
        {
            std::cout << "Register to write:";
            std::cin >> reg_count_read;
            Ask1.reg_count = reg_count_read;
            std::cout << "Data to write to this register:";
            std::cin >> message;
            // get register number to set        
            reg_cin = std::stoi(message);
            // Bytes are negated because 0 cannot be send (Server is stoping receiving then). U2 coding - negation + 1 on server side
            Ask1.offset.data_t[1] = ~(reg_cin >> 8);
            Ask1.offset.data_t[0] = ~reg_cin;
        }

        //Writing exact bytes to modbus struct

        char data_heap_crc[5] = { Ask1.address, Ask1.function,
        Ask1.offset.data_t[1],Ask1.offset.data_t[0], Ask1.reg_count };
        int length_crc = sizeof(data_heap_crc);
        Ask1.crc.data_u = CRC(data_heap_crc, length_crc, CRCTable);




        reg_cin = std::stoi(message);
        if (message == "q") {
            break;
        }

        std::string Modbus_frame = "";
        // puting every byte in one string (UDP function requires string argument)
        Modbus_frame = Modbus_frame + Ask1.address + Ask1.function +
            Ask1.offset.data_t[1] + Ask1.offset.data_t[0] +
            Ask1.reg_count +
            Ask1.crc.data_t[1] + Ask1.crc.data_t[0];

        for (char c : Modbus_frame) {
            // divide char to 2 x HEX
            unsigned char firstNibble = (c >> 4) & 0x0F;
            unsigned char secondNibble = c & 0x0F;

            // Hex in console
            std::cout << std::hex << std::setw(1) << static_cast<unsigned int>(firstNibble);
            std::cout << std::hex << std::setw(1) << static_cast<unsigned int>(secondNibble);
        }

        std::cout << "\n";
        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == -1) {
            std::cerr << "Error sending data." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // 
    }

    std::this_thread::sleep_for(std::chrono::seconds(1)); //1 sec delay between asking


    receiveThread.join();

    close(clientSocket);

    return 0;
}




unsigned short int CRC(char* data, int length, unsigned int crcTable[256])
{
    unsigned int crc = 0xFFFF;
    for (int i = 0; i < length; i++) {
        crc = (crc << 8) ^ crcTable[((crc >> 8) ^ data[i]) & 0xFF];
    }
    return crc;
}

unsigned short int CRC_vector(const char* data, int length, unsigned int crcTable[256])
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