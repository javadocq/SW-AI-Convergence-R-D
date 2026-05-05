#ifndef ADAPTIVE_THREAD_POOL_C_HPP
#define ADAPTIVE_THREAD_POOL_C_HPP

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>

class AdaptiveThreadPoolC {
public:
    using Task = std::function<void()>;
    struct TaskWrapper {
        Task task;
        std::chrono::steady_clock::time_point enqueue_time;
    };

    // 생성자 
    AdaptiveThreadPoolC(size_t min_thread_size, size_t max_thread_size);

    // 소멸자
    ~AdaptiveThreadPoolC();

    template<class F, class... Args>
    void submit(F&& f, Args&&... args) {
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(mtx_);
            tasks_.push({ [task]() { task(); }, std::chrono::steady_clock::now() });
        }
        cv_.notify_one();
    }
    
    // 스레드 종료
    void shutdown();

    // Metrics getters
    size_t get_max_thread_count() const { return max_thread_count.load(); }
    size_t get_thread_create_count() const { return thread_create_count.load(); }
    size_t get_thread_destroy_count() const { return thread_destroy_count.load(); }
    size_t get_resize_count() const { return resize_count.load(); }
    uint64_t get_total_idle_time_ms() const { return total_idle_time_ms.load(); }

    // New Metrics getters
    uint64_t get_total_queue_wait_time_ms() const { return total_queue_wait_time_ms.load(); }
    uint64_t get_total_service_time_ms() const { return total_service_time_ms.load(); }
    uint64_t get_total_latency_ms() const { return total_latency_ms.load(); }
    size_t get_completed_task_count() const { return completed_task_count.load(); }
    uint64_t get_total_thread_lifetime_ms() const { return total_thread_lifetime_ms.load(); }

private:
    // 스레드 생성
    void create_worker(); 
    // 큐 상태 모니터링 루프
    void monitor_loop();

    std::vector<std::thread> bees_;
    std::queue<TaskWrapper> tasks_;
    std::thread monitor_;

    std::mutex mtx_;
    std::condition_variable cv_;

    size_t min_thread_size;
    size_t max_thread_size;

    std::atomic<size_t> active_thread;  

    // Metrics
    std::atomic<size_t> max_thread_count{0};
    std::atomic<size_t> thread_create_count{0};
    std::atomic<size_t> thread_destroy_count{0};
    std::atomic<size_t> resize_count{0};
    std::atomic<uint64_t> total_idle_time_ms{0};

    // New Metrics
    std::atomic<uint64_t> total_queue_wait_time_ms{0};
    std::atomic<uint64_t> total_service_time_ms{0};
    std::atomic<uint64_t> total_latency_ms{0};
    std::atomic<size_t> completed_task_count{0};
    std::atomic<uint64_t> total_thread_lifetime_ms{0};

    std::atomic<bool> stop_;
};

#endif