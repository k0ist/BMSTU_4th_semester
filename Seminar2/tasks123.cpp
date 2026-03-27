#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>


void task1() {
    long long n = 100000000;
    auto start = std::chrono::high_resolution_clock::now();

    long long sum = 0;
    for (long long i = 1; i <= n; ++i) sum += i;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Время выполнения: " << duration.count() << " миллисекунд" << std::endl;
}

void task2(int seconds) {
    using namespace std::chrono_literals;
    for (int i = seconds; i > 0; --i) {
        std::cout << "Осталось: " << i << " секунд" << std::endl;
        std::this_thread::sleep_for(1s);
    }
    std::cout << "Время вышло!" << std::endl;
}

void task3(int total_seconds) {
    std::chrono::seconds s{total_seconds};
    auto h = std::chrono::duration_cast<std::chrono::hours>(s);
    s -= h;
    auto m = std::chrono::duration_cast<std::chrono::minutes>(s);
    s -= m;

    std::cout << h.count() << " час " << m.count() << " минута " << s.count() << " секунда" << std::endl;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    task1();
    task2(3); // Я сделал ввод внутри программы, через консоль - избыточная нагрузка на проц
    task3(3600);
    return 0;
}