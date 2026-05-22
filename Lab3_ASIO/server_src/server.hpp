#pragma once
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include <memory>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, boost::asio::io_context& io_context);
    void start();

private:
    void do_read();
    void handle_request(const std::string& request);
    void process_echo(const std::string& msg);
    void process_factorial(int num);
    void process_timer(int seconds);

    void do_write(const std::string& response);
    void log_message(const std::string& message);

    tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer_;
    std::vector<std::string> log_;

    std::string write_buffer_;

    static constexpr size_t max_length = 1024;
    char data_[max_length];
};


class MultiThreadedServer {
public:
    MultiThreadedServer(boost::asio::io_context& io_context, short port, int num_threads);

    ~MultiThreadedServer();

    void start();
    void stop();

private:
    void do_accept();

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;

    int num_threads_;
    boost::thread_group thread_group_;
};
