#ifndef ADAPTIVE_THREAD_POOL_HPP
#define ADAPTIVE_THREAD_POOL_HPP

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>

class AdaptiveThreadPool {
public:
    using Task = std::function<void()>;
    struct TaskWrapper {
        Task task;
        std::chrono::steady_clock::time_point enqueue_time;
    };

    // 생성자 
    AdaptiveThreadPool(size_t min_thread_size, size_t max_thread_size);

    // 소멸자
    ~AdaptiveThreadPool();

    template<class F, class... Args>
    void submit(F&& f, Args&&... args) {
        // 함수랑 인자를 하나로 묶어주는 bind 사용
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(mtx_);
            // 큐에 작업 추가
            tasks_.push({ [task]() { task(); }, std::chrono::steady_clock::now() });

            // if(tasks_.size() > active_thread) 임계점의 효율을 늘리기 위해 * 2로 설정
            if(tasks_.size() > (active_thread * 2) && active_thread < max_thread_size) {
                create_worker();
            }
        }

        // 하나의 스레드를 가동
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

    // 스레드 배열
    std::vector<std::thread> bees_;
    std::queue<TaskWrapper> tasks_;

    std::mutex mtx_;
    std::condition_variable cv_;

    size_t min_thread_size;
    size_t max_thread_size;

    // 현재 가동중인 스레드 수
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

    bool stop_;
};

#endif