#include <wolv/net/socket_server.hpp>

#include <wolv/utils/guards.hpp>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <fcntl.h>

#if defined(OS_WINDOWS)
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/time.h>
#endif

namespace wolv::net {

    SocketServer::SocketServer(u16 port, size_t bufferSize, i32 maxClientCount, bool localOnly)
        : m_bufferSize(bufferSize), m_maxClientCount(maxClientCount), m_threadPool(maxClientCount), m_localOnly(localOnly) {
        initializeSockets();

        this->m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (this->m_socket == SocketNone)
            return;

        auto guard = SCOPE_GUARD {
            closeSocket(this->m_socket);
            this->m_socket = SocketNone;
        };

        struct sockaddr_in serverAddr = {};
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family       = AF_INET;
        serverAddr.sin_addr.s_addr  = htonl(this->m_localOnly ? INADDR_LOOPBACK : INADDR_ANY);
        serverAddr.sin_port         = htons(port);

        int bindResult = ::bind(this->m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        if (bindResult < 0) {
            this->m_error = bindResult;
            return;
        }

        constexpr int reuse = true;
        #if defined (OS_WINDOWS)
            setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&reuse), sizeof(reuse));
            setsockopt(this->m_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char *>(&reuse), sizeof(reuse));

            u_long mode = 1;
            ::ioctlsocket(this->m_socket, FIONBIO, &mode);
        #else
            #ifdef SO_REUSEPORT
                setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const void *>(&reuse), sizeof(reuse));
            #else
                setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const void *>(&reuse), sizeof(reuse));
            #endif

            ::fcntl(this->m_socket, F_SETFL, ::fcntl(this->m_socket, F_GETFL, 0) | O_NONBLOCK);
        #endif

        int listenResult = ::listen(this->m_socket, this->m_maxClientCount);
        if (listenResult < 0) {
            this->m_error = listenResult;
            return;
        }

        guard.release();
    }

    SocketServer::~SocketServer() {
        this->shutdown();
    }

    SocketServer::SocketServer(wolv::net::SocketServer &&other) noexcept {
        this->m_socket = other.m_socket;
        other.m_socket = SocketNone;

        this->m_bufferSize = other.m_bufferSize;
        this->m_maxClientCount = other.m_maxClientCount;
        this->m_localOnly = other.m_localOnly;
        this->m_error = other.m_error;

        this->m_threadPool = std::move(other.m_threadPool);
    }

    SocketServer& SocketServer::operator=(wolv::net::SocketServer &&other) noexcept {
        if (this != &other) {
            this->m_socket = other.m_socket;
            other.m_socket = SocketNone;

            this->m_bufferSize = other.m_bufferSize;
            this->m_maxClientCount = other.m_maxClientCount;
            this->m_localOnly = other.m_localOnly;
            this->m_error = other.m_error;

            this->m_threadPool = std::move(other.m_threadPool);
        }

        return *this;
    }

    SocketHandle acceptConnection(SocketHandle serverSocket) {
        struct sockaddr_in clientAddr = {};
        socklen_t clientSize = sizeof(clientAddr);
        return ::accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientSize);
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
            ::setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuse), sizeof(reuse));

            #if defined(OS_WINDOWS)
                u_long mode = 1;
                ::ioctlsocket(clientSocket, FIONBIO, &mode);
            #else
                ::fcntl(clientSocket, F_SETFL, ::fcntl(clientSocket, F_GETFL, 0) | O_NONBLOCK);
            #endif

            int receivedByteCount = ::recv(clientSocket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0);
            if (receivedByteCount > 0) {
                std::copy_n(buffer.begin(), receivedByteCount, std::back_inserter(data));
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