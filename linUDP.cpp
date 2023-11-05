//////////////
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <chrono>

#define CLIENT_PORT 1777
#define SERVER_PORT 4077
#define SERVER_IP "192.168.3.247"

//UDP modbus client template for linux

// Another thread for receiving data via udp
void ReceiveThread(int clientSocket) {
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
            std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

int main() {
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
        int bytesSent = sendto(clientSocket, Modbus_frame.c_str(), Modbus_frame.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error while sending the data: " << WSAGetLastError() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // 
    }

        std::this_thread::sleep_for(std::chrono::seconds(1)); //1 sec delay between asking

    
    receiveThread.join();

    close(clientSocket);

    return 0;
}
