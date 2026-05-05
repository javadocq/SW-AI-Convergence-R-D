#include "../include/adaptive_thread_pool_c.hpp"

// 생성자 구현
AdaptiveThreadPoolC::AdaptiveThreadPoolC(size_t min_thread_size, size_t max_thread_size) 
    :min_thread_size(min_thread_size), max_thread_size(max_thread_size), active_thread(0), stop_(false)
{
    std::unique_lock<std::mutex> lock(mtx_);
    for(size_t i = 0; i < min_thread_size; i++) {
        create_worker();
    }
    monitor_ = std::thread(&AdaptiveThreadPoolC::monitor_loop, this);
}

// 소멸자 구현
AdaptiveThreadPoolC::~AdaptiveThreadPoolC() {
    shutdown();
}

// 모니터링: 큐에 작업이 많은 상태가 3초 이상 지속되면 스레드 증가
void AdaptiveThreadPoolC::monitor_loop() {
    int high_queue_duration_ms = 0;
    const int CHECK_INTERVAL_MS = 100;

    while(!stop_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_INTERVAL_MS));
        if (stop_) break;

        bool should_create = false;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            // 큐의 길이가 스레드 수의 2배 이상인 상태를 "high"로 정의
            if (tasks_.size() > (active_thread * 2)) {
                high_queue_duration_ms += CHECK_INTERVAL_MS;
            } else {
                high_queue_duration_ms = 0; // 상태가 해제되면 초기화
            }

            // 3초 이상 지속되었는지 확인
            if (high_queue_duration_ms >= 3000 && active_thread < max_thread_size) {
                should_create = true;
                high_queue_duration_ms = 0; // 증가 후 쿨타임 또는 초기화를 위해 0으로 설정
            }
        }

        if (should_create) {
            std::unique_lock<std::mutex> lock(mtx_);
            if (active_thread < max_thread_size) {
                create_worker();
            }
        }
    }
}

// 스레드 생성
void AdaptiveThreadPoolC::create_worker() {
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
            Task task;
            std::chrono::steady_clock::time_point enqueue_time;
            {
                std::unique_lock<std::mutex> lock(mtx_);

                auto start_idle = std::chrono::steady_clock::now();
                // Adaptive-C: idle 상태가 10초 이상 지속되면 스레드 감소
                bool timeout = !cv_.wait_for(lock, std::chrono::seconds(10), [this] {
                    return stop_.load() || !tasks_.empty();
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
void AdaptiveThreadPoolC::shutdown() {
    stop_ = true;
    cv_.notify_all();

    if(monitor_.joinable()) {
        monitor_.join();
    }

    for(std::thread &bee_ : bees_) {
        if(bee_.joinable()) {
            bee_.join();
        }
    }
}