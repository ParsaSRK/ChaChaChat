#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>

#include "net/socket.hpp"

static SocketError map_errno(int e) {
    switch (e) {
        case EAGAIN:
            return {SocketErrorType::WouldBlock, e};
        case EPIPE:
        case ECONNRESET:
            return {SocketErrorType::ConnectionReset, e};
        case ETIMEDOUT:
            return {SocketErrorType::TimedOut, e};
        case EINTR:
            return {SocketErrorType::Interrupted, e};
        default:
            return {SocketErrorType::Unknown, e};
    }
}

struct Socket::Impl {
    int fd = -1;
};

Socket::Socket(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {
}

std::expected<Socket, SocketError> Socket::connect(std::string_view host,
                                                   uint16_t port) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_ADDRCONFIG;

    char port_str[6];
    std::snprintf(port_str, sizeof(port_str), "%u", port);

    addrinfo* result = nullptr;
    std::string host_str(host);  // to ensure NUL-termination
    int gai_ret = ::getaddrinfo(host_str.c_str(), port_str, &hints, &result);
    if (gai_ret != 0) {
        return std::unexpected(
            SocketError{SocketErrorType::ResolutionFailed, gai_ret});
    }

    int fd = -1;
    int last_errno = 0;

    for (addrinfo* p = result; p != nullptr; p = p->ai_next) {
        fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) {
            last_errno = errno;
            continue;
        }

        if (::connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;

        last_errno = errno;
        ::close(fd);
        fd = -1;
    }

    ::freeaddrinfo(result);

    if (fd < 0) {
        return std::unexpected(map_errno(last_errno));
    }

    auto impl = std::make_unique<Impl>();
    impl->fd = fd;

    return Socket(std::move(impl));
}

std::expected<void, SocketError> Socket::send(std::span<const std::byte> data) {
    if (!impl_ || impl_->fd < 0) {
        return std::unexpected(SocketError{SocketErrorType::InvalidSocket, 0});
    }

    std::size_t total = 0;
    while (total < data.size()) {
        ssize_t temp = ::send(impl_->fd, data.data() + total,
                              data.size() - total, MSG_NOSIGNAL);
        if (temp < 0) {
            if (errno == EINTR) continue;
            return std::unexpected(map_errno(errno));
        }

        if (temp == 0) {
            return std::unexpected(
                SocketError{SocketErrorType::ConnectionReset, 0});
        }

        total += static_cast<std::size_t>(temp);
    }

    return {};
}
std::expected<std::size_t, SocketError> Socket::recv(
    std::span<std::byte> data) {
    if (!impl_ || impl_->fd < 0) {
        return std::unexpected(SocketError{SocketErrorType::InvalidSocket, 0});
    }

    while (true) {
        ssize_t n = ::recv(impl_->fd, data.data(), data.size(), 0);

        if (n < 0) {
            if (errno == EINTR) continue;
            return std::unexpected(map_errno(errno));
        }

        if (n == 0) {
            return std::unexpected(
                SocketError{SocketErrorType::ConnectionClosed, 0});
        }

        return static_cast<std::size_t>(n);
    }
}

void Socket::close() noexcept {
    if (impl_ && impl_->fd >= 0) {
        ::close(impl_->fd);
        impl_->fd = -1;
    }
}

Socket::Socket(Socket&&) noexcept = default;
Socket& Socket::operator=(Socket&&) noexcept = default;

Socket::~Socket() noexcept {
    close();
}
