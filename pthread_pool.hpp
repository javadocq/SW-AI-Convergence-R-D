#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <pthread.h>
#include <vector>
#include <iostream>

class ThreadPool {
public:
    enum State { ON, OFF, STANDBY };
    enum SubmitFlag { POOL_WAIT, POOL_NOWAIT };
    enum ShutdownMode { POOL_COMPLETE, POOL_DISCARD }; 

    const int POOL_SUCCESS = 0;
    const int POOL_FAIL = -1;
    const int POOL_FULL = -2;

    // 생성자 
    ThreadPool(size_t bee_size, size_t queue_size);

    // 소멸자
    virtual ~ThreadPool();

    int submit(void (*f)(void*), void* p, SubmitFlag flag);
    int shutdown(ShutdownMode how);

protected:
    // pthread_create에 전달하기 위한 정적 래퍼 함수
    static void* worker_entry(void* param);
    void worker_loop(); 

    struct Task {
        void (*function)(void*);
        void* param;
    };

    // 기존 C 구조체의 필드들
    std::vector<Task> queue_; 
    std::vector<pthread_t> bees_;

    size_t q_front_;
    size_t q_len_;
    size_t q_size_;
    size_t bee_size_;
    State state_;

    pthread_mutex_t mutex_;
    pthread_cond_t full_;   
    pthread_cond_t empty_;  
}; 

#endif