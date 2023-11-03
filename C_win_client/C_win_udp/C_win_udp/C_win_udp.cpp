#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include "Modbus.h"
#include <iomanip> 

#pragma comment(lib, "ws2_32.lib")

#define CLIENT_PORT 777
#define SERVER_PORT 7
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
                response1.crc.data_t[0] = buffer[data_inkrement*2 + 5];
                response1.crc.data_t[1] = buffer[data_inkrement*2 + 6];
                //Uzaleznic dlugosc odczytywane wektora data od zapytania (ilosc rejestrow) 
                //Tutaj dopisac funkcje CRC check - jezeli CRC sie nie zgadza to zwraca blad
                char crc_resp_comp_data[] = { response1.address + response1.function +
                    response1.data[0].data_t[1] + response1.data[0].data_t[0] };
                int length_crc_comp = sizeof(crc_resp_comp_data);

                unsigned int response_i = response1.data[0].data_t[1] + response1.data[0].data_t[0];

                unsigned short int crc_response_comparison = CRC(crc_resp_comp_data, length_crc_comp, CRCTable);
                bool b_crc_response_comparison = (crc_response_comparison == response1.crc.data_u) ? true : false;
                //chwilowy dodatek - bez sprawdzania crc, bool zawsze true
                b_crc_response_comparison = true;
                if (b_crc_response_comparison)
                {
                    for (data_inkrement = 0; data_inkrement < response1.data_count; data_inkrement++)
                    {
                        std::cout << "Response: " << response1.data[data_inkrement].data_u << std::endl;
                    }
                    
                    
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
    char c = 136;
    std::cout << c << "\n";
    //unsigned int CRCTable[256];
    //ModbusInit(CRCTable);
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
    clientAddr.sin_port = htons(CLIENT_PORT); // Wybierz dowolny numer portu klienta
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    // Przypisz numer portu do gniazda klienta
    if (bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR) {
        std::cerr << "Error in client port" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT); // Numer portu serwera
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
        Ask1.reg_count.data_u = reg_count_read;
        //std::cin >> reg_count_read;
        std::cout << "Register offset reading start";
        std::string message;
        std::cin >> message;
        // get register offset
        unsigned short int reg_cin;
        reg_cin = std::stoi(message);

        //Writing exact bytes to modbus struct
        Ask1.offset.data_u = reg_cin;
        char data_heap_crc[6] = { Ask1.address, Ask1.function,
        Ask1.offset.data_t[1],Ask1.offset.data_t[0], Ask1.reg_count.data_t[1],Ask1.reg_count.data_t[0] };
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
            Ask1.reg_count.data_t[1] + Ask1.reg_count.data_t[0] + 
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

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Odczekaj 1 sekundę przed kolejnym wprowadzeniem wiadomości
    }

    // Thread for receiving responses from server
    receiveThread.join();

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
