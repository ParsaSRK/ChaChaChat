#include <iostream>
#include <string>

#include "net/socket.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    auto connection = Socket::connect("localhost", 5000);
    if (!connection) {
        std::cout << "Connection Failed!" << std::endl;
        return -1;
    }

    std::string message{"Hello From ChaChaChat"};

    if (!connection->send(std::as_bytes(std::span(message)))) {
        std::cout << "Sending Failed!\n" << std::endl;
        return -1;
    }

    std::array<std::byte, 4096> buffer;
    auto result = connection->recv(buffer);
    if (!result) {
        std::cout << "Recieving Failed!\n" << std::endl;
        return -1;
    }

    std::string recieved(reinterpret_cast<char*>(buffer.data()),
                         result.value());

    std::cout << "Recieved: " << recieved << std::endl;

    return 0;
}
