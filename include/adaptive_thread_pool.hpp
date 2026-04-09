#ifndef ADAPTIVE_THREAD_POOL_HPP
#define ADAPTIVE_THREAD_POOL_HPP

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>

class AdativeThreadPool {
public:
    using Task = std::function<void()>;

    // 생성자 
    AdativeThreadPool(size_t min_thread_size, size_t max_thread_size);

    // 소멸자
    ~AdativeThreadPool();

    template<class F, class... Args>
    void submit(F&& f, Args&&... args) {
        // 함수랑 인자를 하나로 묶어주는 bind 사용
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args));

        {
            std::unique_lock<std::mutext> lock(mtx_);
            // 큐에 작업 추가
            tasks_.push([task]() { task(); });

            if(tasks_.size() > active_thread && active_thread < max_thread_size) {
                create_worker();
            }
        }

        // 하나의 스레드를 가동
        cv_.notify_one();

    }
    // 스레드 종료
    void shutdown();

private:
    // 스레드 생성
    void create_worker(); 

    // 스레드 배열
    std::vector<std::thread> bees_;
    std::queue<Task> tasks_;

    std::mutex mtx_;
    std::condition_variable cv_;

    size_t min_thread_size;
    size_t max_thread_size;

    // 현재 가동중인 스레드 수
    std::atomic<size_t> active_thread;  

    bool stop_;
};

#endif