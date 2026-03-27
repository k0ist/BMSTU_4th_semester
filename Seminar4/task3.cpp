#include <iostream>
#include <vector>
#include <thread>
#include <semaphore>


template<typename T>
class SemaphoreBuffer {
private:
    int K;
    std::vector<std::vector<T>> buffers;
    std::vector<std::unique_ptr<std::counting_semaphore<100>>> empty;
    std::vector<std::unique_ptr<std::counting_semaphore<100>>> full;
    std::vector<std::mutex> mtxs;

public:
    SemaphoreBuffer(int n_buffers, int cap) : K(n_buffers), buffers(n_buffers), mtxs(n_buffers) {
        for (int i = 0; i < K; ++i) {
            empty.push_back(std::make_unique<std::counting_semaphore<100>>(cap));
            full.push_back(std::make_unique<std::counting_semaphore<100>>(0));
        }
    }

    void produce(T value, int idx, int timeout_ms) {
        if (empty[idx]->try_acquire_for(std::chrono::milliseconds(timeout_ms))) {
            std::lock_guard<std::mutex> lock(mtxs[idx]);
            buffers[idx].push_back(value);
            std::cout << "Thread " << std::this_thread::get_id() << " Produce to B:" << idx << std::endl;
            full[idx]->release();
        }
        std::this_thread::yield();
    }

    T consume(int idx, int timeout_ms) {
        if (full[idx]->try_acquire_for(std::chrono::milliseconds(timeout_ms))) {
            std::lock_guard<std::mutex> lock(mtxs[idx]);
            T val = buffers[idx].back();
            buffers[idx].pop_back();
            std::cout << "Thread " << std::this_thread::get_id() << " Consume from B:" << idx << std::endl;
            empty[idx]->release();
            return val;
        }
        std::this_thread::yield();
        throw std::runtime_error("Empty");
    }
};