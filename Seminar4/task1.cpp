#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <semaphore>
#include <atomic>
#include <algorithm>
#include <condition_variable>

template<typename T>
class ResourcePool {
private:
    std::vector<T> resources;
    std::counting_semaphore<100> sem;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> failed_attempts{0};
    struct Request {
        int priority;
        std::thread::id tid;
        bool operator<(const Request& other) const { return priority < other.priority; }
    };
    std::vector<Request> wait_queue;

public:
    ResourcePool(std::vector<T> initial) : resources(std::move(initial)), sem(resources.size()) {}

    T acquire(int priority, int timeout_ms) {
        auto start = std::chrono::steady_clock::now();
        std::unique_lock<int> fake_lock;

        {
            std::lock_guard<std::mutex> lock(mtx);
            wait_queue.push_back({priority, std::this_thread::get_id()});
            std::push_heap(wait_queue.begin(), wait_queue.end());
        }

        if (sem.try_acquire_for(std::chrono::milliseconds(timeout_ms))) {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Thread " << std::this_thread::get_id() << " (P:" << priority << ") ACQUIRE" << std::endl;
            T res = resources.back();
            resources.pop_back();
            auto it = std::find_if(wait_queue.begin(), wait_queue.end(),
                                   [](const Request& r){ return r.tid == std::this_thread::get_id(); });
            if (it != wait_queue.end()) {
                wait_queue.erase(it);
                std::make_heap(wait_queue.begin(), wait_queue.end());
            }
            return res;
        }

        failed_attempts++;
        std::lock_guard<std::mutex> lock(mtx);
        auto it = std::find_if(wait_queue.begin(), wait_queue.end(),
                               [](const Request& r){ return r.tid == std::this_thread::get_id(); });
        if (it != wait_queue.end()) {
            wait_queue.erase(it);
            std::make_heap(wait_queue.begin(), wait_queue.end());
        }
        throw std::runtime_error("Timeout");
    }

    void release(T res) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            resources.push_back(res);
            std::cout << "Thread " << std::this_thread::get_id() << " RELEASE" << std::endl;
        }
        sem.release();
        std::this_thread::yield();
    }

    void add_resource(T res) {
        std::lock_guard<std::mutex> lock(mtx);
        resources.push_back(res);
        sem.release();
    }
};