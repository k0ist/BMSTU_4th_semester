#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <boost/asio.hpp>
#include <windows.h>

using boost::asio::ip::tcp;

class Client : public std::enable_shared_from_this<Client> {
public:
    Client(boost::asio::io_context& io_context)
            : socket_(io_context) {}

    void start(const tcp::resolver::results_type& endpoints) {
        do_connect(endpoints);
    }

    void send_message(const std::string& message) {
        auto self(shared_from_this());

        auto tracking_message = std::make_shared<std::string>(message);

        boost::asio::async_write(socket_, boost::asio::buffer(*tracking_message),
                                 [this, self, tracking_message](boost::system::error_code ec, std::size_t) {
                                     if (!ec) {
                                         do_read();
                                     } else {
                                         std::cerr << "Ошибка отправки: " << ec.message() << std::endl;
                                     }
         });
    }

private:
    void do_connect(const tcp::resolver::results_type& endpoints) {
        auto self(shared_from_this());
        boost::asio::async_connect(socket_, endpoints,
                                   [this, self](boost::system::error_code ec, tcp::endpoint) {
                                       if (!ec) {
                                           std::cout << "Успешно подключено к серверу! Введите сообщение: " << std::endl;
                                           do_read();
                                       } else {
                                           std::cerr << "Ошибка подключения: " << ec.message() << std::endl;
                                       }
                                   });
    }

    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, self](boost::system::error_code ec, std::size_t length) {
                                    if (!ec) {
                                        std::cout << "Ответ: " << std::string(data_, length) << std::endl;
                                        do_read();
                                    } else {
                                        std::cerr << "Соединение потеряно или ошибка чтения: " << ec.message() << std::endl;
                                    }
                                });
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "12345");

        auto client = std::make_shared<Client>(io_context);
        client->start(endpoints);

        std::thread t([&io_context]() { io_context.run(); });

        std::string message;
        while (std::getline(std::cin, message) && !message.empty()) {
            client->send_message(message);
        }

        io_context.stop();
        if (t.joinable()) {
            t.join();
        }
    } catch (std::exception& e) {
        std::cerr << "Исключение в main: " << e.what() << std::endl;
    }
    return 0;
}