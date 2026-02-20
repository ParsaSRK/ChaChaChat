#pragma once

#include <cstddef>
#include <expected>
#include <memory>
#include <span>
#include <string_view>

enum class SocketErrorType {
    TimedOut,
    WouldBlock,
    ConnectionReset,
    Interrupted,
    Unknown,
    ResolutionFailed,
    InvalidSocket,
    ConnectionClosed,
};

struct SocketError {
    SocketErrorType type;
    int nativeCode;
};

class Socket {
public:
    [[nodiscard]] static std::expected<Socket, SocketError> connect(
        std::string_view host, uint16_t port);
    [[nodiscard]] std::expected<void, SocketError> send(
        std::span<const std::byte> data);
    [[nodiscard]] std::expected<size_t, SocketError> recv(
        std::span<std::byte> data);

    void close() noexcept;

    Socket(Socket&&) noexcept;
    Socket& operator=(Socket&&) noexcept;

    ~Socket() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    explicit Socket(std::unique_ptr<Impl> impl);

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
};
