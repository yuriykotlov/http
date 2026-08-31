#include <iostream>
#include <format>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif

    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif

int main(){
    WSADATA wsaData{};
    
    int result{ WSAStartup(MAKEWORD(2,2), &wsaData) };

    if(result != 0){
        std::cout << std::format("WSAStartup failed: {}", result);

        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}