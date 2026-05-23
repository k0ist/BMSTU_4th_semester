#include <boost/asio.hpp>
#include <iostream>
#include <exception>
#include <windows.h>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

asio::awaitable<void> echo_session(tcp::socket sock) {
    char data[1024];
    try {
        for (;;) {
            auto [ec, n] = co_await sock.async_read_some(
                    asio::buffer(data),
                    asio::as_tuple(asio::use_awaitable)
            );

            if (ec == asio::error::eof) {
                break;
            }
            if (ec) {
                throw boost::system::system_error(ec);
            }

            co_await asio::async_write(sock, asio::buffer(data, n), asio::use_awaitable);
        }
    } catch (const std::exception& e) {
        std::cerr << "[Сессия] Ошибка: " << e.what() << std::endl;
    }
}

asio::awaitable<void> listener(tcp::acceptor acceptor) {
    try {
        for (;;) {
            tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
            auto executor = acceptor.get_executor();
            asio::co_spawn(executor, echo_session(std::move(socket)), asio::detached);
        }
    } catch (const std::exception& e) {
        std::cerr << "[Слушатель] Ошибка: " << e.what() << std::endl;
    }
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    try {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9090));
        asio::co_spawn(io_context, listener(std::move(acceptor)), asio::detached);

        std::cout << "Однопоточный Эхо-сервер запущен на порту 8080..." << std::endl;
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "[Main] Критическая ошибка: " << e.what() << std::endl;
    }
    return 0;
}