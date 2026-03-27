#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class MatrixProcessor {
private:
    std::vector<std::vector<int>>& matrix;
    size_t n_threads;
    std::mutex mtx;
    std::condition_variable cv;
    size_t active_threads;

public:
    MatrixProcessor(std::vector<std::vector<int>>& m, size_t n)
            : matrix(m), n_threads(n), active_threads(0) {}

    void apply(std::function<int(int)> func) {
        size_t rows = matrix.size();
        active_threads = n_threads;

        for (size_t t = 0; t < n_threads; ++t) {
            std::thread([this, t, rows, func]() {
                std::cout << "Start Thread: " << std::this_thread::get_id() << std::endl;
                for (size_t i = t; i < rows; i += n_threads) {
                    for (size_t j = 0; j < matrix[i].size(); ++j) {
                        matrix[i][j] = func(matrix[i][j]);
                        if (j % 10 == 0) std::this_thread::yield();
                    }
                }
                std::cout << "End Thread: " << std::this_thread::get_id() << std::endl;
                std::lock_guard<std::mutex> lock(mtx);
                active_threads--;
                if (active_threads == 0) cv.notify_one();
            }).detach();
        }
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return active_threads == 0; });
    }
};

int main() {
    std::vector<std::vector<int>> m = {{1, 2}, {3, 4}};
    MatrixProcessor mp(m, 2);
    mp.apply([](int x) { return x * 2; });
    return 0;
}