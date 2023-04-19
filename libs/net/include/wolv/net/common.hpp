#include <wolv/utils/guards.hpp>

#if defined(OS_WINDOWS)

    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

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

    inline void closeSocket(SocketHandle socket) {
        #if defined(OS_WINDOWS)
            ::closesocket(socket);
        #else
            ::close(socket);
        #endif
    }

    inline void initializeSockets() {
        #if defined(OS_WINDOWS)
            static bool initialized = false;

            if (initialized)
                return;

            AT_FIRST_TIME {
                WSAData wsa = { };
                WSAStartup(MAKEWORD(2, 2), &wsa);
                initialized = true;
            };

            AT_FINAL_CLEANUP {
                WSACleanup();
                initialized = false;
            };
        #endif
    }

}