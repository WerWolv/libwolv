#pragma once

#include <wolv/types.hpp>
#include <wolv/net/common.hpp>
#include <wolv/utils/thread_pool.hpp>

#include <functional>
#include <string>
#include <thread>
#include <list>

namespace wolv::net {

    class SocketServer {
    public:
        SocketServer() = default;
        explicit SocketServer(u16 port, size_t bufferSize = 1024, i32 maxClientCount = 5, bool localOnly = true);

        using Callback = std::function<std::vector<u8>(SocketHandle, const std::vector<u8>)>;

        void accept(const Callback &callback);

    private:
        void handleClient(SocketHandle clientSocket, const std::atomic<bool> &shouldStop, const Callback &callback) const;

    private:
        size_t m_bufferSize = 1024;
        i32 m_maxClientCount = 5;
        bool m_localOnly = true;

        SocketHandle m_socket = SocketNone;
        util::ThreadPool m_threadPool = util::ThreadPool(0);
    };

}