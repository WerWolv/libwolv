#pragma once

#include <wolv/types.hpp>
#include <wolv/net/common.hpp>
#include <wolv/utils/thread_pool.hpp>

#include <functional>
#include <optional>
#include <string>
#include <thread>
#include <list>

namespace wolv::net {

    class SocketServer {
    public:
        SocketServer() = default;
        explicit SocketServer(u16 port, size_t bufferSize = 1024, i32 maxClientCount = 5, bool localOnly = true);
        ~SocketServer();
        SocketServer(const SocketServer&) = delete;
        SocketServer(SocketServer&&) noexcept;

        SocketServer& operator=(const SocketServer&) = delete;
        SocketServer& operator=(SocketServer&&) noexcept;

        using ReadCallback = std::function<std::vector<u8>(SocketHandle, const std::vector<u8>)>;
        using CloseCallback = std::function<void(SocketHandle)>;

        void accept(const ReadCallback &callback, const CloseCallback &closeCallback = nullptr, bool keepAlive = false);

        void send(SocketHandle socket, const std::vector<u8> &data) const;
        void send(SocketHandle socket, const std::string &data) const;

        void shutdown();

        [[nodiscard]] std::optional<int> getError() const;
        [[nodiscard]] bool isListening() const;
        [[nodiscard]] bool isActive() const;

    private:
        void handleClient(SocketHandle clientSocket, bool keepAlive, const std::atomic<bool> &shouldStop, const ReadCallback &callback) const;

    private:
        size_t m_bufferSize = 1024;
        i32 m_maxClientCount = 5;
        bool m_localOnly = true;

        SocketHandle m_socket = SocketNone;
        util::ThreadPool m_threadPool = util::ThreadPool(0);
        std::optional<int> m_error;
    };

}