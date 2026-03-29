#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <semaphore>
#include <mutex>
#include <windows.h>


std::counting_semaphore<100> sem(0);
std::mutex cout_mtx;

void order_processor(int id) {
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "Заказ №" << id << " обработан сотрудником " << std::this_thread::get_id() << "\n";
    }
    sem.release();
}

void order_deliverer(int id) {
    sem.acquire();
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "Заказ № " << id << " доставлен сотрудником " << std::this_thread::get_id() << "\n";
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    boost::thread_group workers;
    const int orders = 10;

    for (int i = 0; i < orders; ++i) {
        workers.create_thread(boost::bind(order_processor, i));
        workers.create_thread(boost::bind(order_deliverer, i));
    }

    workers.join_all();
}