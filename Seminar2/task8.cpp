#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>

struct Task {
    int value;
    int priority;
    int duration_ms;
    int steps;
    int currentStep = 0;

    int calculate() const { return value * value; }
};

class VirtualThread {
    std::vector<Task> tasks;
public:
    void addTask(const Task& t) { tasks.push_back(t); }
    bool hasTasks() const { return !tasks.empty(); }

    void runStep(int threadId) {
        if (tasks.empty()) return;

        // Поиск задачи с максимальным приоритетом
        auto it = std::max_element(tasks.begin(), tasks.end(),
                                   [](const Task& a, const Task& b) { return a.priority < b.priority; });

        it->currentStep++;
        std::cout << "Виртуальный поток " << threadId << " выполняет шаг "
                  << it->currentStep << "/" << it->steps << " задачи "
                  << it->value << " с приоритетом " << it->priority << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(it->duration_ms / it->steps));

        if (it->currentStep >= it->steps) {
            std::cout << "Виртуальный поток " << threadId << " завершил задачу "
                      << it->value << ": результат = " << it->calculate() << std::endl;
            tasks.erase(it);
        }
    }
};

class HyperThreadingSimulator {
    VirtualThread vt1, vt2;
public:
    void addTaskTo1(const Task& t) { vt1.addTask(t); }
    void addTaskTo2(const Task& t) { vt2.addTask(t); }

    void execute() {
        while (vt1.hasTasks() || vt2.hasTasks()) {
            if (vt1.hasTasks()) vt1.runStep(1);
            if (vt2.hasTasks()) vt2.runStep(2);
        }
    }
};

int main() {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> valDist(1, 50), durDist(200, 1000), prioDist(1, 10), stepDist(2, 5);

    HyperThreadingSimulator sim;
    for (int i = 0; i < 3; ++i) {
        sim.addTaskTo1({valDist(gen), prioDist(gen), durDist(gen), stepDist(gen)});
        sim.addTaskTo2({valDist(gen), prioDist(gen), durDist(gen), stepDist(gen)});
    }

    sim.execute();
    return 0;
}