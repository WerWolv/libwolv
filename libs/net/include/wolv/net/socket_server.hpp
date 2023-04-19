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
        SocketServer(size_t bufferSize = 1024, i32 maxClientCount = 5);

        using Callback = std::function<std::vector<u8>(SocketHandle, const std::vector<u8>)>;

        void listen(u16 port, const Callback &callback);
        void stop();

    private:
        void handleClient(SocketHandle clientSocket, const std::atomic<bool> &shouldStop, const Callback &callback);

    private:
        size_t m_bufferSize = 1024;
        i32 m_maxClientCount = 5;

        SocketHandle m_socket = SocketNone;
        util::ThreadPool m_threadPool;

        std::atomic<bool> m_running = false;
    };

}