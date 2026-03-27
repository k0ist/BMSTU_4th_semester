#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class Buffer {
private:
    std::vector<T> buffer;
    size_t capacity;
    std::mutex mtx;
    std::condition_variable cv_prod, cv_cons;

public:
    Buffer(size_t cap) : capacity(cap) {}

    void produce(T value) {
        std::unique_lock<std::mutex> lock(mtx);
        while (buffer.size() >= capacity) {
            std::this_thread::yield();
            cv_prod.wait(lock);
        }
        buffer.push_back(value);
        std::cout << "ID: " << std::this_thread::get_id() << " Produced: " << value << std::endl;
        cv_cons.notify_one();
    }

    T consume() {
        std::unique_lock<std::mutex> lock(mtx);
        while (buffer.empty()) {
            std::this_thread::yield();
            cv_cons.wait(lock);
        }
        T value = buffer.back();
        buffer.pop_back();
        std::cout << "ID: " << std::this_thread::get_id() << " Consumed: " << value << std::endl;
        cv_prod.notify_one();
        return value;
    }
};

int main() {
    Buffer<int> buf(5);
    for(int i=0; i<3; ++i) std::thread([&](){ buf.produce(rand()%100); }).detach();
    for(int i=0; i<3; ++i) std::thread([&](){ buf.consume(); }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}