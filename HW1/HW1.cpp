#include <iostream>
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <random>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

struct Account {
    int id;
    double balance;
    std::mutex mtx;

    Account(int i, double b) : id(i), balance(b) {}
};

class BankSystem {
private:
    std::deque<Account> accounts;
    std::atomic<double> total_bank_balance{0.0};
    std::atomic<int> completed_transactions{0};
    std::mutex log_mtx;
    std::ofstream log_file;
    const int max_transactions;

public:
    BankSystem(int num_accounts, double initial_sum, int max_tx)
            : max_transactions(max_tx), log_file("transactions.log") {
        for (int i = 0; i < num_accounts; ++i) {
            accounts.emplace_back(i, initial_sum);
            total_bank_balance = total_bank_balance + initial_sum;
        }
    }

    void process_transactions() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, accounts.size() - 1);
        std::uniform_real_distribution<> amount_dis(1.0, 100.0);

        while (completed_transactions < max_transactions) {
            int from_idx = dis(gen);
            int to_idx = dis(gen);

            if (from_idx == to_idx) continue;

            double amount = amount_dis(gen);
            Account& from_acc = accounts[from_idx];
            Account& to_acc = accounts[to_idx];

            {
                std::scoped_lock lock(from_acc.mtx, to_acc.mtx, log_mtx);

                if (completed_transactions >= max_transactions) break;

                if (from_acc.balance >= amount) {
                    from_acc.balance -= amount;
                    to_acc.balance += amount;
                    completed_transactions++;

                    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    log_file << "[" << std::put_time(std::localtime(&now), "%H:%M:%S") << "] "
                             << "Transfer " << std::fixed << std::setprecision(2) << amount
                             << " from ID " << from_acc.id << " to ID " << to_acc.id
                             << " | Balances: " << from_acc.balance << ", " << to_acc.balance << std::endl;
                }
            }
            boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
        }
    }

    void print_results(double elapsed_time) {
        std::cout << "--- Final Results ---" << std::endl;
        double current_total = 0;
        for (const auto& acc : accounts) {
            std::cout << "Account ID " << acc.id << ": " << acc.balance << std::endl;
            current_total += acc.balance;
        }
        std::cout << "----------------------" << std::endl;
        std::cout << "Initial Total: " << total_bank_balance << std::endl;
        std::cout << "Final Total:   " << current_total << std::endl;
        std::cout << "Transactions:  " << completed_transactions << std::endl;
        std::cout << "Time Elapsed:  " << elapsed_time << " seconds" << std::endl;
    }
};

int main() {
    const int num_accounts = 10;
    const double initial_balance = 1000.0;
    const int total_tx = 100;
    const int num_threads = 5;

    BankSystem bank(num_accounts, initial_balance, total_tx);

    auto start_time = std::chrono::high_resolution_clock::now();

    boost::thread_group threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.create_thread(boost::bind(&BankSystem::process_transactions, &bank));
    }

    threads.join_all();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    bank.print_results(duration.count());

    return 0;
}