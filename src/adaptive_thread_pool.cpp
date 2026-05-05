#include "../include/adaptive_thread_pool.hpp"
#include <thread>

// 생성자 구현
AdaptiveThreadPool::AdaptiveThreadPool(size_t min_thread_size, size_t max_thread_size) 
    :min_thread_size(min_thread_size), max_thread_size(max_thread_size), active_thread(0), stop_(false)
{

    std::unique_lock<std::mutex> lock(mtx_);
    for(int i = 0; i < min_thread_size; i++) {
        create_worker();
    }
}

// 소멸자 구현
AdaptiveThreadPool::~AdaptiveThreadPool() {
    shutdown();
}

// 스레드 생성
void AdaptiveThreadPool::create_worker() {
    active_thread++;
    thread_create_count++;
    resize_count++;
    
    size_t current_active = active_thread.load();
    size_t current_max = max_thread_count.load();
    while (current_active > current_max && !max_thread_count.compare_exchange_weak(current_max, current_active)) {
    }

    auto thread_start_time = std::chrono::steady_clock::now();

    bees_.emplace_back([this, thread_start_time]() {
        while(true) {
            AdaptiveThreadPool::Task task;
            std::chrono::steady_clock::time_point enqueue_time;
            {
                std::unique_lock<std::mutex> lock(mtx_);

                auto start_idle = std::chrono::steady_clock::now();
                bool timeout = !cv_.wait_for(lock, std::chrono::seconds(5), [this] {
                    return stop_ || !tasks_.empty();
                });
                auto end_idle = std::chrono::steady_clock::now();
                total_idle_time_ms += std::chrono::duration_cast<std::chrono::milliseconds>(end_idle - start_idle).count();

                if(timeout) {
                    if(active_thread > min_thread_size) {
                        active_thread--;
                        thread_destroy_count++;
                        resize_count++;
                        auto thread_end_time = std::chrono::steady_clock::now();
                        total_thread_lifetime_ms += std::chrono::duration_cast<std::chrono::milliseconds>(thread_end_time - thread_start_time).count();
                        return; 
                    }
                    continue;
                }

                if(stop_ && tasks_.empty()) {
                    auto thread_end_time = std::chrono::steady_clock::now();
                    total_thread_lifetime_ms += std::chrono::duration_cast<std::chrono::milliseconds>(thread_end_time - thread_start_time).count();
                    return;
                }

                auto task_wrapper = std::move(tasks_.front());
                task = std::move(task_wrapper.task);
                enqueue_time = task_wrapper.enqueue_time;
                tasks_.pop();
            }

            if (task) {
                auto dequeue_time = std::chrono::steady_clock::now();
                total_queue_wait_time_ms += std::chrono::duration_cast<std::chrono::milliseconds>(dequeue_time - enqueue_time).count();
                
                task();
                
                auto complete_time = std::chrono::steady_clock::now();
                total_service_time_ms += std::chrono::duration_cast<std::chrono::milliseconds>(complete_time - dequeue_time).count();
                total_latency_ms += std::chrono::duration_cast<std::chrono::milliseconds>(complete_time - enqueue_time).count();
                completed_task_count++;
            }
        }
    });
}

// 종료
void AdaptiveThreadPool::shutdown() {
    {
        // unique_lock은 자신을 감싸고 있는 중괄호가 끝날 때까지만 lock을 걸어놓는다.   
        std::unique_lock<std::mutex> lock(mtx_);
        if(stop_) return;
        stop_ = true;
    }


    for(std::thread &bee_ : bees_) {
        if(bee_.joinable()) {
            bee_.join();
        }
    }
}