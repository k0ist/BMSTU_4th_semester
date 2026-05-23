#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <iostream>
#include <string>
#include <variant>
#include <optional>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using namespace boost::asio::experimental::awaitable_operators;

asio::awaitable<std::optional<std::string>> read_from(tcp::socket& sock, std::string name) {
    char data[1024];
    auto [ec, n] = co_await sock.async_read_some(asio::buffer(data), asio::as_tuple(asio::use_awaitable));

    if (ec == asio::error::eof) {
        co_return std::nullopt;
    }
    if (ec) {
        throw boost::system::system_error(ec);
    }
    co_return "[" + name + "]: " + std::string(data, n);
}

asio::awaitable<void> multiplexer(tcp::socket sock1, tcp::socket sock2) {
    bool sock1_open = true;
    bool sock2_open = true;

    try {
        while (sock1_open || sock2_open) {
            if (sock1_open && sock2_open) {
                auto result = co_await (read_from(sock1, "sock1") || read_from(sock2, "sock2"));

                if (result.index() == 0) {
                    auto res = std::get<0>(result);
                    if (!res) {
                        sock1_open = false;
                        std::cout << "[Система] Сокет 1 закрылся." << std::endl;
                    } else {
                        std::cout << *res << std::flush;
                    }
                } else {
                    auto res = std::get<1>(result);
                    if (!res) {
                        sock2_open = false;
                        std::cout << "[Система] Сокет 2 закрылся." << std::endl;
                    } else {
                        std::cout << *res << std::flush;
                    }
                }
            }
            else if (sock1_open) {
                auto res = co_await read_from(sock1, "sock1");
                if (!res) sock1_open = false;
                else std::cout << *res << std::flush;
            }
            else if (sock2_open) {
                auto res = co_await read_from(sock2, "sock2");
                if (!res) sock2_open = false;
                else std::cout << *res << std::flush;
            }
        }
        std::cout << "[Система] Оба сокета закрыты. Выход." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\nОшибка: " << e.what() << std::endl;
    }
}