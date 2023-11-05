#include <wolv/net/socket_client.hpp>

#include <iterator>
#include <fcntl.h>

namespace wolv::net {

    SocketClient::SocketClient(Type type) : m_type(type) {
        initializeSockets();
    }

    SocketClient::SocketClient(SocketClient &&other) noexcept {
        this->m_socket    = other.m_socket;
        this->m_connected = other.m_connected;

        other.m_socket = SocketNone;
    }

    SocketClient::~SocketClient() {
        this->disconnect();
    }

    SocketClient& SocketClient::operator=(wolv::net::SocketClient &&other) noexcept {
        this->m_socket    = other.m_socket;
        this->m_connected = other.m_connected;

        other.m_socket = SocketNone;

        return *this;
    }

    void SocketClient::writeBytes(const std::vector<u8> &bytes) const {
        if (!this->isConnected()) return;

        ::send(this->m_socket, reinterpret_cast<const char *>(bytes.data()), bytes.size(), 0);
    }

    void SocketClient::writeString(const std::string &string) const {
        if (!this->isConnected()) return;

        ::send(this->m_socket, string.c_str(), string.length(), 0);
    }

    std::vector<u8> SocketClient::readBytes(size_t size) const {
        std::vector<u8> data;
        data.resize(size);

        #if defined(OS_WINDOWS)
            u_long mode = 1;
            ::ioctlsocket(this->m_socket, FIONBIO, &mode);
        #else
            ::fcntl(this->m_socket, F_SETFL, ::fcntl(clientSocket, F_GETFL, 0) | O_NONBLOCK);
        #endif

        auto receivedSize = ::recv(this->m_socket, reinterpret_cast<char *>(data.data()), size, 0);

        if (receivedSize < 0)
            return {};

        data.resize(receivedSize);

        return data;
    }

    std::string SocketClient::readString(size_t size) const {
        auto bytes = readBytes(size);

        std::string result;
        std::copy(bytes.begin(), bytes.end(), std::back_inserter(result));

        return result;
    }

    bool SocketClient::isConnected() const {
        return this->m_connected;
    }

    void SocketClient::connect(const std::string &address, u16 port) {
        this->m_socket = ::socket(AF_INET, this->m_type == Type::TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
        if (this->m_socket == SocketNone)
            return;

        sockaddr_in client = { };

        client.sin_family = AF_INET;
        client.sin_port   = htons(port);
        client.sin_addr.s_addr = ::inet_addr(address.c_str());

        this->m_connected = ::connect(this->m_socket, reinterpret_cast<sockaddr *>(&client), sizeof(client)) == 0;
    }

    void SocketClient::disconnect() {
        if (this->m_socket != SocketNone) {
            closeSocket(this->m_socket);
        }

        this->m_connected = false;
    }

}