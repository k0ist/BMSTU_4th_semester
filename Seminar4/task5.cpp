#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <semaphore>
#include <atomic>
#include <chrono>


struct Task {
    int id;
    int required_slots;
    int duration_ms;
    int priority;
    bool operator<(const Task& o) const { return priority < o.priority; }
};

class TaskScheduler {
private:
    std::priority_queue<Task> queue;
    std::counting_semaphore<100> res_sem;
    std::mutex mtx;
    std::atomic<int> completed{0};

public:
    TaskScheduler(int total_res) : res_sem(total_res) {}

    void submit(Task t) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(t);
    }

    inline void execute_task(Task& t) {
        std::cout << "Thread " << std::this_thread::get_id() << " Executing T:" << t.id << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(t.duration_ms));
    }

    void worker() {
        while (true) {
            Task t;
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (queue.empty()) break;
                t = queue.top();
                queue.pop();
            }

            for(int i=0; i<t.required_slots; ++i) res_sem.acquire();
            execute_task(t);
            for(int i=0; i<t.required_slots; ++i) res_sem.release();

            completed++;
            std::this_thread::yield();
        }
    }
};