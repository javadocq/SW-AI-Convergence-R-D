#include "pthread_pool.hpp"
#include <iostream>
#include <unistd.h> 

int main() {
    // 1. 객체 생성 (생성자가 pthread_pool_init을 대신함)
    // 일꾼 4명, 큐 사이즈 10
    ThreadPool pool(4, 10);

    std::cout << "[Main] Thread Pool initialized with 4 workers." << std::endl;

    // 2. 작업 제출 (Implicit 방식: 람다 활용)
    for (int i = 0; i < 5; i++) {
        // [i]는 외부 변수 i를 람다 안으로 복사해서 가져오겠다는 뜻입니다.
        pool.submit([i]() {
            std::cout << "[Task " << i << "] is running on thread: " 
                      << pthread_self() << std::endl;
            
            // 실제 작업 시뮬레이션 (0.5초)
            usleep(500000); 
            
            std::cout << "[Task " << i << "] is finished!" << std::endl;
        });
    }

    // 작업들이 처리될 시간을 잠시 기다려줌
    sleep(2);

    // 3. 종료 (소멸자가 자동으로 처리하지만 명시적으로 호출 가능)
    std::cout << "[Main] Requesting shutdown..." << std::endl;
    pool.shutdown(ThreadPool::POOL_COMPLETE);

    return 0;
}