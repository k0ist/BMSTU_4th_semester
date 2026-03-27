#include <iostream>
#include <queue>
#include <thread>
#include <semaphore>
#include <string>

struct Job {
    std::string doc;
    int priority;
    bool operator<(const Job& o) const { return priority < o.priority; }
};

class PrinterQueue {
private:
    std::priority_queue<Job> jobs;
    std::counting_semaphore<10> sem;
    std::mutex mtx;

public:
    PrinterQueue(int n) : sem(n) {}

    void printJob(std::string doc, int priority, int timeout_ms) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            jobs.push({doc, priority});
        }

        if (sem.try_acquire_for(std::chrono::milliseconds(timeout_ms))) {
            Job current;
            {
                std::lock_guard<std::mutex> lock(mtx);
                current = jobs.top();
                jobs.pop();
            }
            std::cout << "Thread " << std::this_thread::get_id() << " Printing: " << current.doc << " (P:" << priority << ")" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            sem.release();
        } else {
            std::cout << "Thread " << std::this_thread::get_id() << " Timeout/Interrupted" << std::endl;
        }
        std::this_thread::yield();
    }
};