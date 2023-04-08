#pragma once

#include <wolv/types.hpp>

#include <string>
#include <vector>

#if defined(OS_WINDOWS)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

    #define SOCKET_NONE INVALID_SOCKET
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>

    #define SOCKET_NONE -1
#endif

namespace wolv::util {

    class Socket {
    public:
        enum class Type : u32 {
            TCP = 0,
            UDP = 1
        };

        Socket() = default;
        Socket(Type type) : m_type(type) {}
        Socket(const Socket &) = delete;
        Socket(Socket &&other) noexcept;

        Socket(const std::string &address, u16 port, Type type);
        ~Socket();

        Socket& operator=(const Socket &) = delete;
        Socket& operator=(Socket &&other) noexcept;

        void connect(const std::string &address, u16 port);
        void disconnect();

        [[nodiscard]] bool isConnected() const;

        [[nodiscard]] std::string readString(size_t size = 0x1000) const;
        [[nodiscard]] std::vector<u8> readBytes(size_t size = 0x1000) const;

        void writeString(const std::string &string) const;
        void writeBytes(const std::vector<u8> &bytes) const;

    private:
        bool m_connected = false;
        Type m_type = Type::TCP;

        #if defined(OS_WINDOWS)

            SOCKET m_socket = SOCKET_NONE;

        #else

            int m_socket = SOCKET_NONE;

        #endif
    };

}