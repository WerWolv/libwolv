#include <wolv/net/common.hpp>

#include <wolv/utils/guards.hpp>

namespace wolv::net {

    void closeSocket(SocketHandle socket) {
        #if defined(OS_WINDOWS)
            ::closesocket(socket);
        #else
            ::close(socket);
        #endif
    }

    void initializeSockets() {
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