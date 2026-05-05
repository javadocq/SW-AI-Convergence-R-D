#include "../include/adaptive_thread_pool.hpp"
#include "../include/adaptive_thread_pool_b.hpp"
#include "../include/adaptive_thread_pool_c.hpp"
#include "../include/pthread_pool.hpp" 
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

// 공통 워크로드: I/O + CPU
void mixed_workload(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); 

    volatile long long sum = 0; 
    for(long long i = 0; i < 100000000; i++) { // 계산량 조정
        sum += i;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

template<typename PoolType>
void print_metrics(const PoolType& pool) {
    size_t completed = pool.get_completed_task_count();
    double avg_latency = completed > 0 ? (double)pool.get_total_latency_ms() / completed : 0;
    double avg_queue_wait = completed > 0 ? (double)pool.get_total_queue_wait_time_ms() / completed : 0;
    double avg_service = completed > 0 ? (double)pool.get_total_service_time_ms() / completed : 0;
    size_t thread_destroys = pool.get_thread_destroy_count();
    double avg_thread_lifetime = thread_destroys > 0 ? (double)pool.get_total_thread_lifetime_ms() / thread_destroys : 0;

    std::cout << "Max Thread Count: " << pool.get_max_thread_count() << std::endl;
    std::cout << "Thread Create Count: " << pool.get_thread_create_count() << std::endl;
    std::cout << "Thread Destroy Count: " << pool.get_thread_destroy_count() << std::endl;
    std::cout << "Resize Count: " << pool.get_resize_count() << std::endl;
    std::cout << "Total Idle Time (ms): " << pool.get_total_idle_time_ms() << std::endl;
    std::cout << "Completed Tasks: " << completed << std::endl;
    std::cout << "Avg Latency (ms): " << avg_latency << std::endl;
    std::cout << "Avg Queue Wait (ms): " << avg_queue_wait << std::endl;
    std::cout << "Avg Service Time (ms): " << avg_service << std::endl;
    std::cout << "Avg Thread Lifetime (ms): " << avg_thread_lifetime << std::endl;
}

int main() {
    int total_tasks = 50; // 50개의 작업

    std::cout << "==========================================" << std::endl;
    std::cout << "   스레드 풀 성능 비교 실험 (Mixed)       " << std::endl;
    std::cout << "==========================================" << std::endl;

    // 실험 1: Pthread 기반 Static
    std::cout << "\n[Static Mode (4 threads) 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    auto start = std::chrono::system_clock::now();
    {
        ThreadPool pool(4, total_tasks);
        for (int i = 0; i < total_tasks; i++) pool.submit([i]() { mixed_workload(i); });
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << ">>> Static Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 2: Adaptive-A
    std::cout << "\n[Adaptive-A Mode (2, 12 threads) 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    start = std::chrono::system_clock::now();
    {
        AdativeThreadPool pool(2, 12);
        for (int i = 0; i < total_tasks; i++) pool.submit([i]() { mixed_workload(i); });
        std::this_thread::sleep_for(std::chrono::seconds(7)); 
        std::cout << "--- Adaptive-A Metrics ---" << std::endl;
        print_metrics(pool);
    }
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> Adaptive-A Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 3: Adaptive-B
    std::cout << "\n[Adaptive-B Mode (2, 12 threads) 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    start = std::chrono::system_clock::now();
    {
        AdaptiveThreadPoolB pool(2, 12);
        for (int i = 0; i < total_tasks; i++) pool.submit([i]() { mixed_workload(i); });
        std::this_thread::sleep_for(std::chrono::seconds(7));
        std::cout << "--- Adaptive-B Metrics ---" << std::endl;
        print_metrics(pool);
    }
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> Adaptive-B Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 4: Adaptive-C
    std::cout << "\n[Adaptive-C Mode (2, 12 threads) 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    start = std::chrono::system_clock::now();
    {
        AdaptiveThreadPoolC pool(2, 12);
        for (int i = 0; i < total_tasks; i++) pool.submit([i]() { mixed_workload(i); });
        std::this_thread::sleep_for(std::chrono::seconds(12));
        std::cout << "--- Adaptive-C Metrics ---" << std::endl;
        print_metrics(pool);
    }
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> Adaptive-C Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    return 0;
}
