#include <pthread.h>
#include "pthread_pool.hpp"

// 생성자 -> 기존 pthread_pool_init의 역할
ThreadPool::ThreadPool(size_t bee_size, size_t queue_size)
    : q_limit_(queue_size), bee_size_(bee_size), state_(ON)
{
    
    /* 메모리 할당에 실패하면 예외를 던진다. */
    bees_.resize(bee_size);

    /* 뮤텍스 및 조건변수 초기화 */
    pthread_mutex_init(&mutex_, nullptr);
    pthread_cond_init(&full_, nullptr);
    pthread_cond_init(&empty_, nullptr);

    /* 일꾼 스레드 생성 */
    for (int i = 0; i < bee_size; i++) {
        pthread_create(&bees_[i], NULL, worker_entry, this);
    }
}

// 소멸자 -> 안전하게 스레드 종료 및 자원 정리
ThreadPool::~ThreadPool() {
    if(state_ == ON) {
        shutdown(POOL_COMPLETE);
    } 

    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&full_);
    pthread_cond_destroy(&empty_);
}

/* pthread_create는 C 기반이라 클래스 멤버 함수를 직접 못 받으니 포인터를 캐스팅해서 실행 */
void* ThreadPool::worker_entry(void* param) {
    /* 전달 받은 this 포인터를 다시 클래스 객체로 캐스팅해서 실제 루프 실행 */
    static_cast<ThreadPool*>(param)->worker_loop();
    return nullptr;
}

/* 실제 일꾼의 루프 함수 */
void ThreadPool::worker_loop() {
    while(true) {
        Task task;

        /* 뮤텍스 잠금 */
        pthread_mutex_lock(&mutex_);

        /* 대기열이 비어 있고, 여전히 실행 중이면 계속 기다린다. */
        while(tasks_.empty() && state_ == ON) {
            pthread_cond_wait(&full_, &mutex_);
        }

        /* 
         * 1) DISCARD 모드(OFF)로 들어왔을 때: 즉시 빠져나와 종료 
         * 2) COMPLETE 모드(STANDBY)로 들어왔고, 대기열이 비어 있으면 종료 
         */
        if(state_ == OFF or (state_ == STANDBY and tasks_.empty())) {
            pthread_mutex_unlock(&mutex_);
            break;
        }

        /* 대기열에서 작업 꺼내기 */
        task = std::move(tasks_.front());
        tasks_.pop();
        
        /* 빈 자리가 생겼음을 알림 */
        pthread_cond_signal(&empty_);
        pthread_mutex_unlock(&mutex_);

        /* 꺼낸 '함수 객체'를 그냥 호출 (Implicit의 핵심) */
        if (task) {
            task();
        }
    }

    /* 스레드 종료 */
    pthread_exit(nullptr);
}

// 작업 제출
int ThreadPool::submit(Task task) 
{
    /* 1) 뮤텍스 잠금 */
    pthread_mutex_lock(&mutex_);

    /* 2) pool이 이미 종료 중(OFF 또는 STANDBY)이면 작업을 받지 않는다 */
    if (state_ != ON) {
        pthread_mutex_unlock(&mutex_);
        return POOL_FAIL;
    }

    /* 3) 대기열이 가득 찬 상황 처리 */
    if (tasks_.size() == q_limit_) {
        // 여기서는 POOL_WAIT 모드를 기본으로 가정
        while (tasks_.size() == q_limit_ && state_ == ON) {
            pthread_cond_wait(&empty_, &mutex_);
        }
    }

    /* 4) 대기열에 작업 삽입 */
    tasks_.push(std::move(task));

    /* 5) 작업이 채워졌으니, 기다리던 worker 스레드를 깨운다 */
    pthread_cond_signal(&full_);
    pthread_mutex_unlock(&mutex_);

    return POOL_SUCCESS;
}

int ThreadPool::shutdown(ShutdownMode how)
{
    /* 1) 뮤텍스 잠금 */
    pthread_mutex_lock(&mutex_);

    /* 2) 상태 전환: OFF=즉시 종료(버림), STANDBY=대기열 비운 뒤 종료 */
    state_ = (how == POOL_DISCARD ? OFF : STANDBY);

    /* 3) 작업이 없어도 대기 중인 모든 worker와 submit 대기 중인 스레드를 깨운다 */
    pthread_cond_broadcast(&full_);
    pthread_cond_broadcast(&empty_);
    pthread_mutex_unlock(&mutex_);

    /* 4) 모든 worker 스레드를 join */
    for (size_t i = 0; i < bee_size_; i++) {
        pthread_join(bees_[i], nullptr);
    }

    return POOL_SUCCESS;
}

