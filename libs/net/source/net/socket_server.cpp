#include <wolv/net/socket_server.hpp>

#include <wolv/utils/guards.hpp>

namespace wolv::net {

    SocketServer::SocketServer(u16 port, size_t bufferSize, i32 maxClientCount) : m_bufferSize(bufferSize), m_maxClientCount(maxClientCount), m_threadPool(maxClientCount) {
        initializeSockets();

        this->m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (this->m_socket == SocketNone)
            return;

        auto guard = SCOPE_GUARD {
            closeSocket(this->m_socket);
            this->m_socket = SocketNone;
        };

        struct sockaddr_in serverAddr = {};
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family       = AF_INET;
        serverAddr.sin_addr.s_addr  = INADDR_ANY;
        serverAddr.sin_port         = htons(port);

        int bindResult = ::bind(this->m_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bindResult < 0)
            return;

        int listenResult = ::listen(this->m_socket, this->m_maxClientCount);
        if (listenResult < 0)
            return;

        guard.release();
    }

    SocketHandle acceptConnection(SocketHandle serverSocket) {
        struct sockaddr_in clientAddr = {};
        socklen_t clientSize = sizeof(clientAddr);
        return ::accept(serverSocket, (struct sockaddr*)&clientAddr, &clientSize);
    }

    void setSocketTimeout(SocketHandle socket, u32 milliseconds) {
        #if defined(OS_WINDOWS)
            DWORD timeout = milliseconds;
            setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
        #else
            struct timeval timeout = { 0, milliseconds * 1000 };
            setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
        #endif
    }

    void SocketServer::handleClient(SocketHandle clientSocket, const std::atomic<bool> &shouldStop, const Callback &callback) const {
        std::vector<u8> buffer(this->m_bufferSize);
        std::vector<u8> data;

        while (!shouldStop) {
            setSocketTimeout(clientSocket, 100);

            bool reuse = true;
            setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuse), sizeof(reuse));

            int n = ::recv(clientSocket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0);
            if (n <= 0) {
                if (!data.empty()) {
                    std::vector<u8> result = callback(clientSocket, data);
                    ::send(clientSocket, reinterpret_cast<char*>(result.data()), result.size(), 0);
                    data.clear();
                }
                break;
            } else {
                std::copy(buffer.begin(), buffer.begin() + n, std::back_inserter(data));
            }
        }
    }

    void SocketServer::accept(const Callback &callback) {
        auto clientSocket = acceptConnection(this->m_socket);
        if (clientSocket == SocketNone) {
            return;
        }

        this->m_threadPool.enqueue([this, clientSocket, callback](const auto &shouldStop) {
            this->handleClient(clientSocket, shouldStop, callback);
            closeSocket(clientSocket);
        });
    }

}