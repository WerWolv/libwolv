#pragma once

#include <wolv/types.hpp>
#include <wolv/net/common.hpp>

#include <functional>
#include <string>
#include <thread>
#include <vector>

namespace wolv::net {

    class SocketClient {
    public:
        enum class Type : u32 {
            TCP = 0,
            UDP = 1
        };

        explicit SocketClient(Type type = Type::TCP);
        SocketClient(const SocketClient &) = delete;
        SocketClient(SocketClient &&other) noexcept;

        ~SocketClient();

        SocketClient& operator=(const SocketClient &) = delete;
        SocketClient& operator=(SocketClient &&other) noexcept;

        void connect(const std::string &address, u16 port);
        void disconnect();

        [[nodiscard]] bool isConnected() const;

        [[nodiscard]] std::string readString(size_t size = 0x1000) const;
        [[nodiscard]] std::vector<u8> readBytes(size_t size = 0x1000) const;
        [[nodiscard]] int readBytes(u8 *buffer, size_t size) const;

        void writeString(const std::string &string) const;
        void writeBytes(const std::vector<u8> &bytes) const;
        void writeBytes(const u8 *buffer, size_t size) const;

    private:
        bool m_connected = false;
        Type m_type = Type::TCP;

        SocketHandle m_socket = SocketNone;

    };

}