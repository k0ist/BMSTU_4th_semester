#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <windows.h>

enum class Stage { ASSEMBLY, PAINTING, PACKING };

std::mutex mtx;
std::condition_variable cv;
std::atomic<Stage> current_stage{Stage::ASSEMBLY};
const int total_cars = 10;

void assemble() {
    for (int i = 0; i < total_cars; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return current_stage == Stage::ASSEMBLY; });
        std::cout << "[Этап 1] Сборка машины #" << i + 1 << "\n";
        current_stage = Stage::PAINTING;
        cv.notify_all();
    }
}

void paint() {
    for (int i = 0; i < total_cars; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return current_stage == Stage::PAINTING; });
        std::cout << "[Этап 2] Покраска машины #" << i + 1 << "\n";
        current_stage = Stage::PACKING;
        cv.notify_all();
    }
}

void pack() {
    for (int i = 0; i < total_cars; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return current_stage == Stage::PACKING; });
        std::cout << "[Этап 3] Упаковка машины #" << i + 1 << "\n";
        current_stage = Stage::ASSEMBLY;
        cv.notify_all();
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::thread t1(assemble);
    std::thread t2(paint);
    std::thread t3(pack);
    t1.join(); t2.join(); t3.join();
    return 0;
}