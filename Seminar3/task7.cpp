#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class PriorityQueue {
private:
    struct Element {
        T value;
        int priority;
        bool operator<(const Element& other) const { return priority < other.priority; }
    };
    std::priority_queue<Element> pq;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(T val, int prio) {
        std::lock_guard<std::mutex> lock(mtx);
        pq.push({val, prio});
        std::cout << "ID: " << std::this_thread::get_id() << " PUSH: " << val << " (P:" << prio << ")" << std::endl;
        cv.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mtx);
        while (pq.empty()) {
            std::this_thread::yield();
            cv.wait(lock);
        }
        T val = pq.top().value;
        pq.pop();
        std::cout << "ID: " << std::this_thread::get_id() << " POP: " << val << std::endl;
        return val;
    }
};

int main() {
    PriorityQueue<std::string> pq;
    std::thread([&](){ pq.push("Low", 1); pq.push("High", 10); }).detach();
    std::thread([&](){ pq.pop(); pq.pop(); }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}