#if _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "../include/win_client.hpp"
#include "../include/utils.hpp"

#include <iostream>
#include <string>
#include <cstring>

#include <winsock2.h>
#include <ws2tcpip.h>

constexpr uint16_t BUFFER_LEN{512};
static long long time_began{};

// expect args to be 'client localhost'

void win_client(int argc, char* argv[], const char* PORT, const long long timestamp){
    time_began = timestamp;

    if(argc < 2){
        print_message(time_began, "win_client | Args count is less than 2.");
        return;
    }

    print_message(time_began, "### client started ###");

    WSADATA wsaData{};

    int feedback{ WSAStartup(MAKEWORD(2, 2), &wsaData) };
    if (feedback != 0){
        print_message(time_began, "win_client | WSAStartup failed: ", feedback);
        WSACleanup();
        return;
    }

    addrinfo *result{ nullptr }, *ptr{ nullptr }, hints{};

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    feedback = getaddrinfo(argv[2], PORT, &hints, &result);
    if(feedback != 0){
        print_message(time_began, "win_client | getaddrinfo failed: ", feedback);
        WSACleanup();
        return;
    }

    SOCKET new_socket{};

    // find actual address in addrinfo to connect to
    for(ptr = result; ptr != nullptr; ptr->ai_next){
        // create socket
        new_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if(new_socket == INVALID_SOCKET){
            print_message(time_began, "win_client | Listening socket is invalid: ", WSAGetLastError());
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
        print_message(time_began, "win_client | A connection to server was not made.");
        WSACleanup();
        return;
    }

    const char* test_message{ "hi" };

    feedback = send(new_socket, test_message, std::strlen(test_message), 0);
    if(feedback == SOCKET_ERROR){
        print_message(time_began, "win_client | Failed to send message: ", WSAGetLastError());
        closesocket(new_socket); 
        WSACleanup();
        return;
    }

    print_message(time_began, "win_client | Bytes sent: ", feedback);

    // no more data will be sent
    feedback = shutdown(new_socket, SD_SEND);
    if(feedback == SOCKET_ERROR){
        print_message(time_began, "win_client | Failed to shutdown: ", WSAGetLastError());
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
            print_message(time_began, "win_client | Bytes recieved: " + std::string{buffer});
        } else if(recv_result == 0){
            print_message(time_began, "win_client | Connection has been closed.");
        } else{
            print_message(time_began, "win_client | recv could not get: ", WSAGetLastError());
        }
    } while(recv_result > 0);

    closesocket(new_socket);
    WSACleanup();
}

#endif