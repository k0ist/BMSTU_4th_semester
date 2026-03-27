#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <semaphore>
#include <atomic>

struct FileChunk {
    int chunk_id;
    int file_id;
    void download() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
};

class DownloadManager {
private:
    std::queue<FileChunk> chunk_queue;
    std::counting_semaphore<5> active_files_sem;
    std::counting_semaphore<20> chunk_sem;
    std::mutex mtx;
    std::atomic<int> completed_files{0};

public:
    DownloadManager() : active_files_sem(5), chunk_sem(20) {}

    void add_chunk(FileChunk c) {
        std::lock_guard<std::mutex> lock(mtx);
        chunk_queue.push(c);
    }

    inline void process_chunk(FileChunk& c) {
        std::cout << "Thread " << std::this_thread::get_id() << " DL File:" << c.file_id << " Chunk:" << c.chunk_id << std::endl;
        c.download();
    }

    void download_worker() {
        while (true) {
            FileChunk c;
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (chunk_queue.empty()) break;
                c = chunk_queue.front();
                chunk_queue.pop();
            }

            chunk_sem.acquire();
            process_chunk(c);
            chunk_sem.release();
            std::this_thread::yield();
        }
    }
};