#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>

template <typename T>
class Logger {
private:
    std::ofstream log_file;
    std::mutex mtx;

    inline void write_sync(const T& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        std::string out = "ID: " + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())) + " | Msg: " + std::to_string(msg) + "\n";
        std::cout << out;
        if (log_file.is_open()) log_file << out;
    }

public:
    Logger(std::string path) : log_file(path) {}
    ~Logger() { if (log_file.is_open()) log_file.close(); }

    void log(const T& message) {
        write_sync(message);
    }
};

int main() {
    Logger<int> logger("log.txt");
    for(int i=0; i<5; ++i) std::thread([&logger, i](){ logger.log(i * 10); }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}