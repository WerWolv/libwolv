#pragma once

#if defined(OS_WINDOWS)

    #include <winsock2.h>

#else

    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>

#endif

namespace wolv::net {

    #if defined(OS_WINDOWS)
        using SocketHandle = SOCKET;
        constexpr SocketHandle SocketNone = INVALID_SOCKET;
    #else
        using SocketHandle = int;
        constexpr SocketHandle SocketNone = -1;
    #endif

    void closeSocket(SocketHandle socket);
    void initializeSockets();

}