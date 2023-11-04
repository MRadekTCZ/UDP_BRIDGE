#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include "Modbus.h"
#include <iomanip> 
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define CLIENT_PORT 7771
#define SERVER_PORT 777
#define SERVER_IP "192.168.100.201"




void ReceiveThread(SOCKET clientSocket) {

    Modbus_answer response1{};

    char buffer[1024];
    sockaddr_in serverAddr;
    int addrLen = sizeof(serverAddr);

    while (true) {
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddr, &addrLen);
        
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            int data_inkrement = 0;
            //Writing exact bytes to modbus struct
            response1.address = buffer[0];
            response1.function = buffer[1];
            response1.data_count = buffer[2];
            if(response1.data_count != 0)
            {
                //for loop depends on amount of data incoming
                for (data_inkrement = 0; data_inkrement < response1.data_count; data_inkrement++)
                {
                    response1.data[data_inkrement].data_t[1] = buffer[data_inkrement*2 + 3];
                    response1.data[data_inkrement].data_t[0] = buffer[data_inkrement*2 + 4];
                }
                //response1.crc.data_u = CRC_vector(data_count_ptr, length_crc_res, CRCTable);            
                response1.crc.data_t[0] = buffer[data_inkrement*2 + 3];
                response1.crc.data_t[1] = buffer[data_inkrement*2 + 4];
                
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
                        std::cout << "Response: " << response1.data[data_inkrement].data_u << std::endl;
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
            std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

int main() {
    //char c = 136;
    
    //std::cout << c << "\n";
    //unsigned int CRCTable[256];
    //ModbusInit(CRCTable);
    for (int i = 0; i < 10; i++)
    {
        std::cout << 1 + i * 11 + i * i * 7 + i * i * 3;
    }
    Modbus_ask Ask1{};
    Ask1.address = 0x01;
    Ask1.function = 0x03;
    Ask1.offset.data_u = 0x000A;
    
    
    



    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error in opening Winsock." << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error in opening Winsock: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(CLIENT_PORT); 
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    // Port number and socket
    if (bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR) {
        std::cerr << "Error in client port" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT); 
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Error IP." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Thread for server reading
    std::thread receiveThread(ReceiveThread, clientSocket);

    while (true) {
        
        //std::cout << "Amount of register to read";
        unsigned short int reg_count_read;
        reg_count_read = 1;
        
        std::cout << "Number of register to read:";
        std::cin >> reg_count_read;
        Ask1.reg_count = reg_count_read;
        std::cout << "Register offset reading start:";
        std::string message;
        std::cin >> message;
        // get register offset
        unsigned short int reg_cin;
        reg_cin = std::stoi(message);

        //Writing exact bytes to modbus struct
        Ask1.offset.data_t[1] = 0xFF;
        Ask1.offset.data_t[0] = reg_cin;
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
            Ask1.crc.data_t[1]+ Ask1.crc.data_t[0];

        for (char c : Modbus_frame) {
            // divide char to 2 x HEX
            unsigned char firstNibble = (c >> 4) & 0x0F;
            unsigned char secondNibble = c & 0x0F;

            // Hex in console
            std::cout << std::hex << std::setw(1) << static_cast<unsigned int>(firstNibble) ;
            std::cout << std::hex << std::setw(1) << static_cast<unsigned int>(secondNibble);
        }
        std::cout << "\n";
        int bytesSent = sendto(clientSocket, Modbus_frame.c_str(), Modbus_frame.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error while sending the data: " << WSAGetLastError() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // 
    }

    // Thread for receiving responses from server
    receiveThread.join();

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
