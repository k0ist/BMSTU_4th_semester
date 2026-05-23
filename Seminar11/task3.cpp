#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <atomic>
#include <stdexcept>

namespace asio = boost::asio;

class BankAccount {
private:
    int64_t balance_ = 0;
    asio::strand<asio::io_context::executor_type> strand_;

public:
    explicit BankAccount(asio::io_context::executor_type ex)
            : strand_(asio::make_strand(ex)) {}

    asio::awaitable<void> async_deposit(int64_t amount) {
        co_await asio::post(strand_, asio::use_awaitable);[cite: 1]
        balance_ += amount;
    }

    asio::awaitable<void> async_withdraw(int64_t amount) {
        co_await asio::post(strand_, asio::use_awaitable);[cite: 1]
        if (balance_ < amount) {
            throw std::invalid_argument("Insufficient funds");[cite: 1]
        }
        balance_ -= amount;
    }

    asio::awaitable<int64_t> async_get_balance() {
        co_await asio::post(strand_, asio::use_awaitable);[cite: 1]
        co_return balance_;
    }
};

asio::awaitable<void> transaction_worker(BankAccount& account, std::atomic<int64_t>& global_expected, std::atomic<int>& active_coroutines) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> dist(1, 100);

    int64_t local_delta = 0;

    for (int i = 0; i < 10; ++i) {
        int64_t dep_amount = dist(gen);
        co_await account.async_deposit(dep_amount);
        local_delta += dep_amount;

        int64_t wit_amount = dist(gen);
        try {
            co_await account.async_withdraw(wit_amount);
            local_delta -= wit_amount;
        } catch (const std::invalid_argument&) {

        }
    }

    global_expected += local_delta;
    --active_coroutines;
}

int main() {
    try {
        asio::io_context io_context;
        BankAccount account(io_context.get_executor());

        std::atomic<int64_t> global_expected_balance{0};
        const int num_coroutines = 100;
        std::atomic<int> active_coroutines{num_coroutines};

        for (int i = 0; i < num_coroutines; ++i) {
            asio::co_spawn(
                    io_context,
                    transaction_worker(account, global_expected_balance, active_coroutines),
                    asio::detached
            );
        }

        std::vector<std::thread> pool;
        for (int i = 0; i < 4; ++i) {
            pool.emplace_back([&io_context]() { io_context.run(); });
        }

        for (auto& t : pool) {
            t.join();
        }

        int64_t final_balance = 0;
        io_context.restart();

        asio::co_spawn(io_context, [&]() -> asio::awaitable<void> {
            final_balance = co_await account.async_get_balance();
            co_return;
        }, asio::detached);

        io_context.run();

        std::cout << "=== Результаты тестирования ===" << std::endl;
        std::cout << "Активных корутин осталось: " << active_coroutines.load() << std::endl;
        std::cout << "Финальный баланс на счете: " << final_balance << std::endl;
        std::cout << "Ожидаемый расчетный баланс: " << global_expected_balance.load() << std::endl;

        if (final_balance == global_expected_balance.load() && active_coroutines.load() == 0) {
            std::cout << "Успех! Состояние гонки полностью отсутствует благодаря strand." << std::endl;
        } else {
            std::cout << "Ошибка синхронизации!" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
    }
    return 0;
}