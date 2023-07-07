#include <wolv/net/socket_server.hpp>

#include <wolv/utils/guards.hpp>

#include <cstring>

namespace wolv::net {

    SocketServer::SocketServer(u16 port, size_t bufferSize, i32 maxClientCount, bool localOnly)
        : m_bufferSize(bufferSize), m_maxClientCount(maxClientCount), m_threadPool(maxClientCount), m_localOnly(localOnly) {
        initializeSockets();

        this->m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (this->m_socket == SocketNone)
            return;

        const int reuse = true;
        #if defined (OS_WINDOWS)
            setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&reuse), sizeof(reuse));
            setsockopt(this->m_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char *>(&reuse), sizeof(reuse));
        #else
            #ifdef SO_REUSEPORT
                setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const void *>(&reuse), sizeof(reuse));
            #else
                setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const void *>(&reuse), sizeof(reuse));
            #endif
        #endif

        auto guard = SCOPE_GUARD {
            closeSocket(this->m_socket);
            this->m_socket = SocketNone;
        };

        struct sockaddr_in serverAddr = {};
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family       = AF_INET;
        serverAddr.sin_addr.s_addr  = htonl(this->m_localOnly ? INADDR_LOOPBACK : INADDR_ANY);
        serverAddr.sin_port         = htons(port);

        int bindResult = ::bind(this->m_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bindResult < 0) {
            this->m_error = bindResult;
            return;
        }

        int listenResult = ::listen(this->m_socket, this->m_maxClientCount);
        if (listenResult < 0) {
            this->m_error = listenResult;
            return;
        }

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
            struct timeval timeout = { 0, int(milliseconds * 1000U) };
            setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
        #endif
    }

    void SocketServer::send(SocketHandle socket, const std::vector<u8> &data) const {
        ::send(socket, reinterpret_cast<const char*>(data.data()), data.size(), 0);
    }

    void SocketServer::send(SocketHandle socket, const std::string &data) const {
        ::send(socket, data.c_str(), data.size(), 0);
    }

    void SocketServer::handleClient(SocketHandle clientSocket, bool keepAlive, const std::atomic<bool> &shouldStop, const ReadCallback &callback) const {
        std::vector<u8> buffer(m_bufferSize);
        std::vector<u8> data;

        while (!shouldStop) {
            setSocketTimeout(clientSocket, 100);

            bool reuse = true;
            setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuse), sizeof(reuse));

            int receivedByteCount = ::recv(clientSocket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0);
            if (receivedByteCount > 0) {
                std::copy(buffer.begin(), buffer.begin() + receivedByteCount, std::back_inserter(data));
                continue;
            }

            if (!data.empty()) {

                // Callback with the data
                auto result = callback(clientSocket, data);
                if (!result.empty())
                    ::send(clientSocket, reinterpret_cast<const char *>(result.data()), result.size(), 0);

                // Clear the data
                data.clear();

                // If we're not keeping the connection alive, break
                if (!keepAlive)
                    break;

            }

            // Check if the client is still connected
            if (receivedByteCount < 0 && errno == EAGAIN)
                // We need to continue, because the client is still connected
                continue;

            // If the client is not connected, break
            closeSocket(clientSocket);
            break;
        }
    }

    void SocketServer::accept(const ReadCallback &callback, const CloseCallback& closeCallback, bool keepAlive) {
        auto clientSocket = acceptConnection(this->m_socket);
        if (clientSocket == SocketNone) {
            return;
        }

        this->m_threadPool.enqueue([this, clientSocket, callback, closeCallback, keepAlive](const auto &shouldStop) {
            this->handleClient(clientSocket, keepAlive, shouldStop, callback);
            if (closeCallback)
                closeCallback(clientSocket);
            closeSocket(clientSocket);
        });
    }

    std::optional<int> SocketServer::getError() const {
        return this->m_error;
    }

    bool SocketServer::isListening() const {
        return !this->m_error.has_value();
    }

    bool SocketServer::isActive() const {
        return this->m_socket != SocketNone;
    }

    void SocketServer::shutdown() {
        this->m_threadPool.stop();
        closeSocket(this->m_socket);
        this->m_socket = SocketNone;
    }

}