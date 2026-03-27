#include <iostream>
#include <vector>
#include <string>

class VirtualThread {
    std::vector<int> nums = {5, 7, 10, 12};
    std::vector<std::string> tasks = {"Задача A", "Задача B", "Задача C", "Задача D"};
    size_t current = 0;

public:
    bool hasTasks() { return current < nums.size(); }

    void run(int id) {
        if (!hasTasks()) return;
        int n = nums[current];
        long long res = 1;
        for (int i = 1; i <= n; ++i) res *= i;
        std::cout << "Виртуальный поток " << id << " вычисляет " << n << "! = " << res << std::endl;
        current++;
    }

    void runNextTask(int id) {
        if (current >= tasks.size()) return;
        std::cout << "Виртуальный поток " << id << " начал " << tasks[current] << std::endl;
        std::cout << "Виртуальный поток " << id << " закончил " << tasks[current] << std::endl;
        current++;
    }
};

class HyperThreadingSimulator {
    VirtualThread vt1, vt2;
public:
    void execute() {
        while (vt1.hasTasks() || vt2.hasTasks()) {
            vt1.run(1);
            vt2.run(2);
        }
    }
};

int main() {
    HyperThreadingSimulator sim;
    sim.execute();
    return 0;
}