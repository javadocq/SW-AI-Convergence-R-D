#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <pthread.h>
#include <vector>
#include <functional>
#include <queue>

class ThreadPool {
public:
    using Task = std::function<void()>;

    enum State { ON, OFF, STANDBY };
    enum ShutdownMode { POOL_COMPLETE, POOL_DISCARD }; 

    const int POOL_SUCCESS = 0;
    const int POOL_FAIL = -1;
    const int POOL_FULL = -2;

    // 생성자 
    ThreadPool(size_t bee_size, size_t queue_size);

    // 소멸자
    virtual ~ThreadPool();

    int submit(Task task);
    int shutdown(ShutdownMode how);

protected:
    // pthread_create에 전달하기 위한 정적 래퍼 함수
    static void* worker_entry(void* param);
    void worker_loop(); 

    // 기존 C 구조체의 필드들
    std::queue<Task> tasks_; 
    std::vector<pthread_t> bees_;

    size_t q_limit_;
    size_t bee_size_;
    State state_;

    pthread_mutex_t mutex_;
    pthread_cond_t full_;   
    pthread_cond_t empty_;  
}; 

#endif