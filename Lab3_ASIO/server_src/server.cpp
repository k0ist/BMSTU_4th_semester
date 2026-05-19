#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include "server.hpp"


Session::Session(tcp::socket socket, boost::asio::io_context& io_context)
        : socket_(std::move(socket)),
          strand_(io_context.get_executor()),
          timer_(io_context) {}

void Session::start() {
    do_read();
}

void Session::do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::asio::bind_executor(strand_,
                                                       [this, self](boost::system::error_code ec,
                                                               std::size_t length)
    {
        if (!ec) {
            std::string request(data_, length);
            handle_request(request);
        }
    }));
}

void Session::do_write(const std::string& response) {
    auto self(shared_from_this());

    write_buffer_ = response;

    boost::asio::async_write(socket_, boost::asio::buffer(write_buffer_),
                             boost::asio::bind_executor(strand_,
                                                        [this, self](boost::system::error_code ec, std::size_t)
                                                        {
                                                            if (!ec) {
                                                                do_read();
                                                            } else {
                                                                std::cerr << "Ошибка записи: " << ec.message() << std::endl;
                                                            }
                                                        }));
}


void Session::handle_request(const std::string &request) {
    if (request.rfind("таймер", 0) == 0) {
        try {
            size_t idx = request.find_first_of("0123456789");
            if (idx != std::string::npos) {
                int secs = std::stoi(request.substr(idx));
                process_timer(secs);
                return;
            }
        } catch (...) {}
        do_write("Ошибка: неверный формат таймера. Используйте: таймер N\n");
    }
    else if (!request.empty() && std::isdigit(request[0])) {
        try {
            int nums = std::stoi(request);
            process_factorial(nums);
        } catch (...) {
            do_write("Ошибка: не удалось распознать число для факториала.\n");
        }
    }
    else {
        process_echo(request);
    }
}

void Session::process_factorial(int number) {
    auto self(shared_from_this());
    boost::asio::post(socket_.get_executor(), [this, self, number]() {
        if (number < 0) {
            do_write("Ошибка: отрицательное число!\n");
            return;
        }
        uint64_t result = 1;
        for (int i = 2; i <= number; ++i) result *= i;

        std::string res_str = "Факториал: " + std::to_string(result) + "\n";
        log_message(res_str);
        do_write(res_str);
    });
}

void Session::process_timer(int seconds) {
    auto self(shared_from_this());
    timer_.expires_after(std::chrono::seconds(seconds));
    timer_.async_wait(boost::asio::bind_executor(strand_, [this, self, seconds](const boost::system::error_code& ec) {
        if (!ec) {
            std::string msg = "Прошло " + std::to_string(seconds) + " секунд!\n";
            log_message(msg);
            do_write(msg);
        }
    }));
}

void Session::process_echo(const std::string& msg) {
    log_message("Эхо: " + msg + "\n");
    do_write(msg + "\n");
}


void Session::log_message(const std::string& message) {
    boost::asio::post(strand_, [this, message]() {
        log_.push_back(message);
        std::cout << "[LOG]: " << message;
    });
}

MultiThreadedServer::MultiThreadedServer(boost::asio::io_context& io_context, short port, int num_threads)
        : io_context_(io_context),
          acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
          num_threads_(num_threads)
{
    std::cout << "[SERVER] Инициализация сервера на порту " << port
              << " с пулом из " << num_threads_ << " потоков." << std::endl;
}

MultiThreadedServer::~MultiThreadedServer() {
    stop();
}

void MultiThreadedServer::start() {
    do_accept(); // Заводим первое асинхронное принятие соединения

    // Наполняем boost::thread_group рабочими потоками
    for (int i = 0; i < num_threads_; ++i) {
        thread_group_.create_thread([this]() {
            try {
                io_context_.run();
            }
            catch (const std::exception& e) {
                std::cerr << "[SERVER ERROR] Исключение в рабочем потоке: " << e.what() << std::endl;
            }
        });
    }
    std::cout << "[SERVER] Пул из " << num_threads_ << " потоков успешно занят обработкой io_context.\n";
}

void MultiThreadedServer::stop() {
    std::cout << "[SERVER] Остановка сервера..." << std::endl;

    if (acceptor_.is_open()) {
        acceptor_.close();
    }

    io_context_.stop();

    thread_group_.join_all();

    std::cout << "[SERVER] Все потоки остановлены. Сервер завершил работу." << std::endl;
}

void MultiThreadedServer::do_accept() {
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        if (!ec) {
            std::cout << "[SERVER] Новое подключение от: "
                      << socket.remote_endpoint().address().to_string()
                      << ":" << socket.remote_endpoint().port() << std::endl;

            auto session = std::make_shared<Session>(std::move(socket), io_context_);
            session->start();
        } else {
            std::cerr << "[SERVER ERROR] Ошибка async_accept: " << ec.message() << std::endl;
        }

        do_accept();
    });
}