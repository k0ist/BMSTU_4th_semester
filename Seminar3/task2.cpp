#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

template <typename T>
class Account {
public:
    T balance;
    std::mutex mtx;
    Account(T b) : balance(b) {}
};

template <typename T>
class Bank {
private:
    std::vector<std::unique_ptr<Account<T>>> accounts;
    std::condition_variable cv;
    std::mutex bank_mtx;

    inline void execute_transfer(Account<T>& from, Account<T>& to, T amount) {
        from.balance -= amount;
        to.balance += amount;
    }

public:
    Bank(int n, T initial) {
        for (int i = 0; i < n; ++i) accounts.push_back(std::make_unique<Account<T>>(initial));
    }

    void transfer(int from_idx, int to_idx, T amount) {
        std::thread([this, from_idx, to_idx, amount]() {
            Account<T>& a = *accounts[from_idx];
            Account<T>& b = *accounts[to_idx];

            std::unique_lock<std::mutex> lock(a.mtx);
            cv.wait(lock, [&] { return a.balance >= amount; });

            std::lock_guard<std::mutex> lock_to(b.mtx);
            execute_transfer(a, b, amount);

            std::cout << "ID: " << std::this_thread::get_id() << " Transfer " << amount << " from " << from_idx << " to " << to_idx << std::endl;
            cv.notify_all();
        }).detach();
    }

    void show_total() {
        T total = 0;
        for (auto& acc : accounts) total += acc->balance;
        std::cout << "Total Bank Balance: " << total << std::endl;
    }
};

int main() {
    Bank<int> sber(5, 1000);
    sber.transfer(0, 1, 500);
    sber.transfer(1, 2, 300);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    sber.show_total();
    return 0;
}