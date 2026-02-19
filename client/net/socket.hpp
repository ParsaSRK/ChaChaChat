#pragma once

#include <cinttypes>
#include <expected>
#include <span>
#include <string_view>

enum class SocketErrorType {
    Timeout,
};

struct SocketError {
    SocketErrorType type;
    int nativeCode;
};

class Socket {
public:
    Socket();
    ~Socket();

    std::expected<void, SocketError> connect(std::string_view host,
                                             uint16_t port);
    std::expected<size_t, SocketError> send(std::span<const std::byte> data);
    std::expected<size_t, SocketError> recv(std::span<std::byte> data);

private:
    struct impl;
    impl* impl_;
};
