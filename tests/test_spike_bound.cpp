#include "../include/adaptive_thread_pool.hpp"
#include "../include/adaptive_thread_pool_b.hpp"
#include "../include/adaptive_thread_pool_c.hpp"
#include "../include/pthread_pool.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

// 공통 워크로드: 가벼운 작업
void spike_workload(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

// 공통 테스트 시나리오 함수 (총 200개 작업 한 번에 몰림)
template<typename T>
void run_spike_scenario(T& pool, const std::string& mode_name) {
    std::cout << "\n[" << mode_name << " 테스트 시작]" << std::endl;
    
    std::cout << "[Step 1] 갑작스러운 200개 스파이크 투입!" << std::endl;
    for (int i = 0; i < 200; i++) pool.submit([i]() { spike_workload(i); });

    std::cout << "[Step 2] 작업 완료 대기 및 Recovery(Scale-down) 측정 중..." << std::endl;
}

int main() {
    int total_tasks = 200;

    std::cout << "==========================================" << std::endl;
    std::cout << "   스레드 풀 성능 비교 실험 (Spike + Recovery) " << std::endl;
    std::cout << "==========================================" << std::endl;

    // 실험 1: Static
    auto start = std::chrono::system_clock::now();
    {
        ThreadPool pool(4, total_tasks);
        run_spike_scenario(pool, "Static Mode (4 threads)");
    } 
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << ">>> Static Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 2: Adaptive-A (Timeout 5s)
    start = std::chrono::system_clock::now();
    {
        AdativeThreadPool pool(2, 16);
        run_spike_scenario(pool, "Adaptive-A Mode (2~16 threads)");
        std::this_thread::sleep_for(std::chrono::seconds(7)); // Scale down 대기
        std::cout << "--- Adaptive-A Metrics ---" << std::endl;
        print_metrics(pool);
    } 
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> Adaptive-A Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 3: Adaptive-B (Timeout 5s)
    start = std::chrono::system_clock::now();
    {
        AdaptiveThreadPoolB pool(2, 16);
        run_spike_scenario(pool, "Adaptive-B Mode (2~16 threads)");
        std::this_thread::sleep_for(std::chrono::seconds(7)); // Scale down 대기
        std::cout << "--- Adaptive-B Metrics ---" << std::endl;
        print_metrics(pool);
    } 
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> Adaptive-B Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 4: Adaptive-C (Timeout 10s)
    start = std::chrono::system_clock::now();
    {
        AdaptiveThreadPoolC pool(2, 16);
        run_spike_scenario(pool, "Adaptive-C Mode (2~16 threads)");
        std::this_thread::sleep_for(std::chrono::seconds(13)); // Scale down 대기 (10초 이상)
        std::cout << "--- Adaptive-C Metrics ---" << std::endl;
        print_metrics(pool);
    } 
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> Adaptive-C Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    return 0;
}
