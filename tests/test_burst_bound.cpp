#include "../include/adaptive_thread_pool.hpp"
#include "../include/adaptive_thread_pool_b.hpp"
#include "../include/adaptive_thread_pool_c.hpp"
#include "../include/pthread_pool.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

// 공통 워크로드: 계산
void burst_workload(int id) {
    volatile long long sum = 0;
    for(long long i = 0; i < 200000000; i++) {
        sum += i;
    }
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

// 공통 테스트 시나리오 함수 (총 140개 작업)
template<typename T>
void run_burst_scenario(T& pool, const std::string& mode_name) {
    std::cout << "\n[" << mode_name << " 테스트 시작]" << std::endl;
    
    std::cout << "[Step 1] Idle 상태 (2초)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "[Step 2] 1차 Burst (40개 작업 투입)" << std::endl;
    for (int i = 0; i < 40; i++) pool.submit([i]() { burst_workload(i); });

    std::cout << "[Step 3] 소강 상태 대기 (4초)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(4));

    std::cout << "[Step 4] 2차 Burst (100개 작업 투입)" << std::endl;
    for (int i = 0; i < 100; i++) pool.submit([i]() { burst_workload(i); });

    std::cout << "[Step 5] 잔여 작업 완료 대기 중..." << std::endl;
}

int main() {
    int total_tasks = 140; // 40 + 100

    std::cout << "==========================================" << std::endl;
    std::cout << "   스레드 풀 성능 비교 실험 (Burst Traffic) " << std::endl;
    std::cout << "==========================================" << std::endl;

    // 실험 1: Static
    auto start = std::chrono::system_clock::now();
    {
        ThreadPool pool(4, total_tasks);
        run_burst_scenario(pool, "Static Mode (4 threads)");
    } 
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << ">>> Static Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 2: Adaptive-A
    start = std::chrono::system_clock::now();
    {
        AdativeThreadPool pool(2, 12);
        run_burst_scenario(pool, "Adaptive-A Mode (2~12 threads)");
        std::this_thread::sleep_for(std::chrono::seconds(7));
        std::cout << "--- Adaptive-A Metrics ---" << std::endl;
        print_metrics(pool);
    } 
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> Adaptive-A Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 3: Adaptive-B
    start = std::chrono::system_clock::now();
    {
        AdaptiveThreadPoolB pool(2, 12);
        run_burst_scenario(pool, "Adaptive-B Mode (2~12 threads)");
        std::this_thread::sleep_for(std::chrono::seconds(7));
        std::cout << "--- Adaptive-B Metrics ---" << std::endl;
        print_metrics(pool);
    } 
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> Adaptive-B Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 4: Adaptive-C
    start = std::chrono::system_clock::now();
    {
        AdaptiveThreadPoolC pool(2, 12);
        run_burst_scenario(pool, "Adaptive-C Mode (2~12 threads)");
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
