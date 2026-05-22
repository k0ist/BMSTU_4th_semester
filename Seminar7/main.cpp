#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <numeric>
#include <algorithm>
#include <execution>
#include <queue>
#include <chrono>
#include <random>
#include <barrier>
#include <string>

struct TelemetryData {
    int sensor_id;
    uint64_t timestamp;
    double value;
};

template <typename T>
class ConcurrentQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> active_{true};

public:
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(item));
        }
        cv_.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty() || !active_; });
        if (queue_.empty()) {
            return false;
        }
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void deactivate() {
        active_ = false;
        cv_.notify_all();
    }
};

class TelemetrySystem {
private:
    static constexpr int N_SENSORS = 120;
    static constexpr size_t BATCH_SIZE = 1000;

    ConcurrentQueue<TelemetryData> input_queue_;
    ConcurrentQueue<std::string> storage_queue_;
    std::atomic<bool> is_running_{true};
    std::atomic<uint64_t> processed_count_{0};

    std::mutex aggregator_mutex_;
    std::shared_mutex state_mutex_;

    std::vector<std::thread> sensor_threads_;
    std::thread dispatcher_thread_;
    std::thread storage_thread_;

    double global_telemetry_sum_{0.0};

    void sensor_worker(int start_id, int end_id) {
        std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<double> dist(15.5, 145.8);

        while (is_running_) {
            for (int id = start_id; id < end_id; ++id) {
                TelemetryData data{
                        id,
                        static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()),
                        dist(gen)
                };
                input_queue_.push(data);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    void dispatcher_worker() {
        std::vector<TelemetryData> batch;
        batch.reserve(BATCH_SIZE);

        std::barrier phase_barrier(2);

        while (is_running_) {
            TelemetryData data;
            while (batch.size() < BATCH_SIZE && input_queue_.pop(data)) {
                batch.push_back(data);
            }

            if (batch.empty() && !is_running_) {
                break;
            }

            process_batch(batch);

            phase_barrier.arrive_and_wait();
            batch.clear();
        }
    }

    void process_batch(std::vector<TelemetryData>& batch) {
        double batch_sum = std::transform_reduce(
                std::execution::par_unseq,
                batch.begin(), batch.end(),
                0.0,
                std::plus<>(),
                [](const TelemetryData& d) { return d.value; }
        );

        std::sort(
                std::execution::par,
                batch.begin(), batch.end(),
                [](const TelemetryData& a, const TelemetryData& b) { return a.value < b.value; }
        );

        std::vector<double> scan_results(batch.size());
        std::vector<double> input_values(batch.size());

        std::transform(std::execution::seq, batch.begin(), batch.end(), input_values.begin(), [](const TelemetryData& d) {
            return d.value;
        });

        std::inclusive_scan(
                std::execution::par,
                input_values.begin(), input_values.end(),
                scan_results.begin()
        );

        {
            std::unique_lock<std::shared_mutex> lock(state_mutex_);
            global_telemetry_sum_ += batch_sum;
        }

        {
            std::lock_guard<std::mutex> lock(aggregator_mutex_);
            processed_count_ += batch.size();
        }

        thread_local size_t local_batch_counter = 0;
        local_batch_counter++;

        std::string report = "Batch Processed: size=" + std::to_string(batch.size()) +
                             " | Min=" + std::to_string(batch.front().value) +
                             " | Max=" + std::to_string(batch.back().value) +
                             " | Batch Sum=" + std::to_string(batch_sum) +
                             " | Thread Local Batches Run=" + std::to_string(local_batch_counter);

        storage_queue_.push(report);
    }

    void storage_worker() {
        std::string log_entry;
        while (storage_queue_.pop(log_entry)) {
            std::cout << "[Storage/Visualizer] " << log_entry << std::endl;
        }
    }

public:
    void start() {
        storage_thread_ = std::thread(&TelemetrySystem::storage_worker, this);
        dispatcher_thread_ = std::thread(&TelemetrySystem::dispatcher_worker, this);

        int threads_count = 4;
        int sensors_per_thread = N_SENSORS / threads_count;
        for (int i = 0; i < threads_count; ++i) {
            sensor_threads_.emplace_back(
                    &TelemetrySystem::sensor_worker, this,
                    i * sensors_per_thread, (i + 1) * sensors_per_thread
            );
        }
    }

    void stop() {
        is_running_ = false;
        input_queue_.deactivate();

        for (auto& t : sensor_threads_) {
            if (t.joinable()) t.join();
        }

        if (dispatcher_thread_.joinable()) dispatcher_thread_.join();

        storage_queue_.deactivate();
        if (storage_thread_.joinable()) storage_thread_.join();
    }

    double get_global_sum() {
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        return global_telemetry_sum_;
    }

    uint64_t get_processed_count() {
        std::lock_guard<std::mutex> lock(aggregator_mutex_);
        return processed_count_.load();
    }
};

int main() {
    TelemetrySystem system;
    system.start();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    system.stop();

    std::cout << "\n[Final Stats] Total Metrics Processed: " << system.get_processed_count() << std::endl;
    std::cout << "[Final Stats] Accumulated Global Sum: " << system.get_global_sum() << std::endl;

    return 0;
}