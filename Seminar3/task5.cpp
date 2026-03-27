#include <iostream>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename K, typename V>
class Cache {
private:
    std::map<K, V> data;
    std::mutex mtx;
    std::condition_variable cv;

public:
    inline void set(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mtx);
        data[key] = value;
        std::cout << "ID: " << std::this_thread::get_id() << " SET [" << key << ":" << value << "]" << std::endl;
        cv.notify_all();
    }

    inline V get(const K& key) {
        std::unique_lock<std::mutex> lock(mtx);
        while (data.find(key) == data.end()) {
            std::this_thread::yield();
            cv.wait(lock);
        }
        V val = data[key];
        std::cout << "ID: " << std::this_thread::get_id() << " GET [" << key << ":" << val << "]" << std::endl;
        return val;
    }

    void print_all() {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto const& [key, val] : data) std::cout << key << " => " << val << std::endl;
    }
};

int main() {
    Cache<std::string, int> cache;
    std::thread([&](){ cache.get("test"); }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread([&](){ cache.set("test", 42); }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}