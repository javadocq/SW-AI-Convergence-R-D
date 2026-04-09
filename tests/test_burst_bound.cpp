#include "../include/adaptive_thread_pool.hpp"
#include "../include/pthread_pool.hpp"
#include <iostream>
#include <chrono>
#include <thread>

void burst_workload(int id) {
    volatile long long sum = 0;
    for(long long i = 0; i < 500000000; i++) {
        sum += i;
    }
}

// 공통 테스트 시나리오 함수
template<typename T>
void run_burst_scenario(T& pool, const std::string& mode_name) {
    std::cout << "\n[" << mode_name << " 테스트 시작]" << std::endl;
    
    // Step 1: Idle
    std::cout << "[Step 1] Idle 상태 (3초)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Step 2: 1차 Burst
    std::cout << "[Step 2] 1차 Burst (40개 작업 투입)" << std::endl;
    for (int i = 0; i < 40; i++) pool.submit([i]() { burst_workload(i); });

    // Step 3: 소강 상태
    std::cout << "[Step 3] 소강 상태 대기 (5초)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Step 4: 2차 Burst
    std::cout << "[Step 4] 2차 Burst (100개 작업 투입)" << std::endl;
    for (int i = 0; i < 100; i++) pool.submit([i]() { burst_workload(i); });

    std::cout << "[Step 5] 잔여 작업 완료 대기 중..." << std::endl;
}

int main() {
    // 실험 1: Static Mode (4 threads)
    auto s_start = std::chrono::system_clock::now();
    {
        ThreadPool s_pool(4, 140); // 1차 40개 + 2차 100개
        run_burst_scenario(s_pool, "Static Mode (4 threads)");
    } 
    auto s_end = std::chrono::system_clock::now();
    std::chrono::duration<double> s_diff = s_end - s_start;


    // 실험 2: Adaptive Mode (2 ~ 12 threads)
    auto a_start = std::chrono::system_clock::now();
    {
        AdativeThreadPool a_pool(2, 12);
        run_burst_scenario(a_pool, "Adaptive Mode (2~12 threads)");
    } 
    auto a_end = std::chrono::system_clock::now();
    std::chrono::duration<double> a_diff = a_end - a_start;

    std::cout << "\n==========================================" << std::endl;
    std::cout << ">>> Static   총 소요 시간: " << s_diff.count() << "s" << std::endl;
    std::cout << ">>> Adaptive 총 소요 시간: " << a_diff.count() << "s" << std::endl;
    std::cout << "==========================================" << std::endl;

    return 0;
}