#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <windows.h>

using boost::asio::ip::tcp;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);

        tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);
        socket.connect(endpoint);
        std::cout << "Подключено к серверу. Введите команду (сообщение / число / 'таймер N'):\n";

        for (;;) {
            std::string user_input;
            std::getline(std::cin, user_input);
            if (user_input == "exit") break;
            if (user_input.empty()) continue;

            boost::asio::write(socket, boost::asio::buffer(user_input));

            char reply[1024];
            std::fill_n(reply, 1024, 0);

            size_t reply_length = socket.read_some(boost::asio::buffer(reply));

            std::cout << "Ответ от сервера: " << std::string(reply, reply_length);
        }
    } catch (std::exception& e) {
        std::cerr << "Ошибка клиента: " << e.what() << std::endl;
    }
    return 0;
}