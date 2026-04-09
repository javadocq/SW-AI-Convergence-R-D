#include "../include/adaptive_thread_pool.hpp"
#include "../include/pthread_pool.hpp" // 종원님의 Pthread 헤더
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

// 공통 워크로드: I/O + CPU
void mixed_workload(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); 

    volatile long long sum = 0; 
    for(long long i = 0; i < 200000000; i++) {
        sum += i;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    int total_tasks = 100; // 100개의 작업을 투척

    std::cout << "==========================================" << std::endl;
    std::cout << "   스레드 풀 성능 비교 실험 (Benchmark)   " << std::endl;
    std::cout << "==========================================" << std::endl;

    // 실험 1: Pthread 기반 Static (기존 코드)
    std::cout << "\n[" << "Static Mode (4 threads)" << " 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    std::cout << "설정 : Static( 4 )" << std::endl;

    // 시간 초 시작
    auto start = std::chrono::system_clock::now();

    {
        ThreadPool pool(4, total_tasks);

        for (int i = 0; i < total_tasks; i++) {
            pool.submit([i]() { mixed_workload(i); });
        }
    }

    // 시간 초 종료
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = end - start;
    std::cout << ">>> " << "Static Mode (4 threads)" << " 총 소요 시간: " << diff.count() << "s" << std::endl;


    // 실험 2: C++ 기반 Adaptive 
    std::cout << "\n[" << "Adaptive Mode (2, 12 threads)" << " 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    std::cout << "설정 : Min(" << 2 << "), Max(" << 12 << ")" << std::endl;

    // 시간 초 시작
    start = std::chrono::system_clock::now();

    {
        AdativeThreadPool pool(2, 12);

        for (int i = 0; i < total_tasks; i++) {
            pool.submit([i]() { mixed_workload(i); });
        }
    }

    // 시간 초 종료
    end = std::chrono::system_clock::now();

    diff = end - start;

    std::cout << ">>> " << "Adaptive Mode (2, 12 threads)" << " 총 소요 시간: " << diff.count() << "s" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}