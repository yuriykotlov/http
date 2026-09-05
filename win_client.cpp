#if _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "win_client.hpp"

#include <iostream>
#include <format>
#include <string>
#include <cstring>

#include <winsock2.h>
#include <ws2tcpip.h>

constexpr uint16_t BUFFER_LEN{512};

// expect args to be 'client localhost'

void win_client(int &argc, char* argv[], const char* const &PORT){
    if(argc < 2){
        std::cerr << std::format("Server name: {}\n", argv[0]);
        return;
    }
    
    std::cout << "\n### client started ###\n";

    WSADATA wsaData{};

    int feedback{ WSAStartup(MAKEWORD(2, 2), &wsaData) };
    if (feedback != 0){
        std::cerr << std::format("WSAStartup failed: {}\n", feedback);
        WSACleanup();
        return;
    }

    addrinfo *result{ nullptr }, *ptr{ nullptr }, hints{};

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    feedback = getaddrinfo(argv[1], PORT, &hints, &result);
    if(feedback != 0){
        std::cerr << std::format("getaddrinfo failed: {}\n", feedback);
        WSACleanup();
        return;
    }

    SOCKET new_socket{};

    // find actual address in addrinfo to connect to
    for(ptr = result; ptr != nullptr; ptr->ai_next){
        // create socket
        new_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if(new_socket == INVALID_SOCKET){
            std::cerr << std::format("Listening socket is invalid: {}\n", WSAGetLastError());
            WSACleanup();
            continue;
        }

        // connect
        if(connect(new_socket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)) == SOCKET_ERROR){
            closesocket(new_socket);
            new_socket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if(new_socket == INVALID_SOCKET){
        std::cerr << "A connnection to server was not made.\n";
        WSACleanup();
        return;
    }

    const char* test_message{ "hi" };

    feedback = send(new_socket, test_message, std::strlen(test_message), 0);
    if(feedback == SOCKET_ERROR){
        std::cerr << std::format("Failed to send message: {}\n", WSAGetLastError());
        closesocket(new_socket);
        WSACleanup();
        return;
    }

    std::cout << std::format("Bytes sent: {}\n", feedback);

    // no more data will be sent
    feedback = shutdown(new_socket, SD_SEND);
    if(feedback == SOCKET_ERROR){
        std::cerr << std::format("Failed to shutdown: {}\n", WSAGetLastError());
        closesocket(new_socket);
        WSACleanup();
        return;
    }

    int recv_result{};

    char buffer[BUFFER_LEN]{};

    // recieve any data in case being sent back
    do{
        recv_result = recv(new_socket, buffer, BUFFER_LEN, 0);
        if(recv_result > 0){
            std::cout << std::format("Bytes recieved: {}\n", feedback);
        } else if(recv_result == 0){
            std::cout << "Connection has been closed.\n";
        } else{
            std::cerr << std::format("recv could not get: {}\n", WSAGetLastError());
        }
    } while(recv_result > 0);

    closesocket(new_socket);
    WSACleanup();
}

#endif