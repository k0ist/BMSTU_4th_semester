#include <iostream>
#include <vector>
#include <semaphore>
#include <mutex>
#include <chrono>
#include <thread>


class ParkingLot {
private:
    int capacity;
    std::counting_semaphore<500> sem;
    int occupied = 0;
    std::mutex mtx;
public:
    ParkingLot(int cap) : capacity(cap), sem(cap) {}

    void park(bool isVIP, int timeout_ms){
        bool acquired = false;
        if (isVIP) {
            sem.acquire();
            acquired = true;
        } else {
            acquired = sem.try_acquire_for(std::chrono::milliseconds (timeout_ms));
        }
        if (acquired) {
            std::lock_guard<std::mutex> lock(mtx);
            occupied++;
            std::cout << "Thread " << std::this_thread::get_id() << " [" << (isVIP ? "VIP" : "REG")
                      << "] Parked. Free: " << capacity - occupied << std::endl;
            std::this_thread::yield();
        }
    }

    void leave() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            occupied--;
            std::cout << "Thread number " << std::this_thread::get_id() << " left. Free " << capacity - occupied \
 << std::endl;
        }

        sem.release();
        std::this_thread::yield();
    }

    void adjust_capacity(int extra) {
        std::lock_guard<std::mutex> lock(mtx);
        capacity += extra;
        if (extra > 0) sem.release(extra);
    }
};