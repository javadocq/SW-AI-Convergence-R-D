#include "../include/adaptive_thread_pool.hpp"
#include <thread>

// 생성자 구현
AdativeThreadPool::AdativeThreadPool(size_t min_thread_size, size_t max_thread_size) 
    :min_thread_size(min_thread_size), max_thread_size(max_thread_size), active_thread(0), stop_(false)
{

    std::unique_lock<std::mutex> lock(mtx_);
    for(int i = 0; i < min_thread_size; i++) {
        create_worker();
    }
}

// 소멸자 구현
AdativeThreadPool::~AdativeThreadPool() {
    shutdown();
}

// 스레드 생성
void AdativeThreadPool::create_worker() {
    active_thread++;

    bees_.emplace_back([this]() {
        while(true) {
            AdativeThreadPool::Task task;
            {
                std::unique_lock<std::mutex> lock(mtx_);

                if(!cv_.wait_for(lock, std::chrono::seconds(5), [this] {
                    return stop_ || !tasks_.empty();
                })) {
                    if(active_thread > min_thread_size) {
                        active_thread--;
                        return; // 이 스레드 종료
                    }
                    continue;
                }

                if(stop_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            if (task) {
                task();
            }
        }
    });
}

// 종료
void AdativeThreadPool::shutdown() {
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