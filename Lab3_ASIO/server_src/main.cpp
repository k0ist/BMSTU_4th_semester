#include "server.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <windows.h>
#include <string>

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    try {
        int num_threads = 4;
        if (argc > 1) {
            num_threads = std::stoi(argv[1]);
        }

        boost::asio::io_context io_context;

        MultiThreadedServer server(io_context, 12345, num_threads);

        server.start();

        std::cout << "Сервер запущен. Наберите 'exit' для остановки.\n";
        std::string command;
        while (std::cin >> command) {
            if (command == "exit") {
                break;
            }
        }

        server.stop();

    } catch (std::exception& e) {
        std::cerr << "Исключение в main: " << e.what() << std::endl;
    }
    return 0;
}