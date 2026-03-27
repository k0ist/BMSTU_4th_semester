#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <numeric>

template <typename T>
class ParallelSum {
private:
    std::vector<T> data;
    size_t n_threads;
    T total_sum;
    std::mutex mtx;
    std::condition_variable cv;
    size_t completed_threads;

    inline void add_to_total(T partial_sum, std::thread::id id) {
        std::lock_guard<std::mutex> lock(mtx);
        total_sum += partial_sum;
        std::cout << "Thread: " << id << " | Partial sum: " << partial_sum << std::endl;
        completed_threads++;
        if (completed_threads == n_threads) cv.notify_one();
    }

public:
    ParallelSum(std::vector<T> d, size_t n)
            : data(std::move(d)), n_threads(n), total_sum(0), completed_threads(0) {}

    T compute_sum() {
        size_t size = data.size();
        size_t chunk = size / n_threads;

        for (size_t i = 0; i < n_threads; ++i) {
            size_t start = i * chunk;
            size_t end = (i == n_threads - 1) ? size : (i + 1) * chunk;

            std::thread([this, start, end]() {
                T local_sum = 0;
                for (size_t j = start; j < end; ++j) {
                    local_sum += data[j];
                    if (j % 100 == 0) std::this_thread::yield();
                }
                add_to_total(local_sum, std::this_thread::get_id());
            }).detach();
        }

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return completed_threads == n_threads; });
        return total_sum;
    }
};

int main() {
    std::vector<double> v(1000, 1.1);
    ParallelSum<double> ps(v, 4);
    std::cout << "Total Sum: " << ps.compute_sum() << std::endl;
    return 0;
}