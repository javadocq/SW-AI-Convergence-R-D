#include "../include/adaptive_thread_pool.hpp"
#include "../include/adaptive_thread_pool_b.hpp"
#include "../include/adaptive_thread_pool_c.hpp"
#include "../include/pthread_pool.hpp" 
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

// 공통 워크로드: 0.5초 동안 대기 (I/O 시뮬레이션)
void io_workload(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    int total_tasks = 30; // 30개의 작업을 투척

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
            pool.submit([i]() { io_workload(i); });
        }
    }

    // 시간 초 종료
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = end - start;
    std::cout << ">>> " << "Static Mode 총 소요 시간: " << diff.count() << "s" << std::endl;


    // 실험 2: C++ 기반 Adaptive-A (Queue Length 기반)
    std::cout << "\n[" << "Adaptive-A Mode (2, 10 threads)" << " 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    std::cout << "설정 : Min(" << 2 << "), Max(" << 10 << ")" << std::endl;

    start = std::chrono::system_clock::now();

    {
        AdativeThreadPool pool(2, 10);

        for (int i = 0; i < total_tasks; i++) {
            pool.submit([i]() { io_workload(i); });
        }
        
        // 잠시 대기하여 작업 완료 및 스레드 축소(scale-down) 유도
        std::this_thread::sleep_for(std::chrono::seconds(7));

        size_t completed = pool.get_completed_task_count();
        double avg_latency = completed > 0 ? (double)pool.get_total_latency_ms() / completed : 0;
        double avg_queue_wait = completed > 0 ? (double)pool.get_total_queue_wait_time_ms() / completed : 0;
        double avg_service = completed > 0 ? (double)pool.get_total_service_time_ms() / completed : 0;
        size_t thread_destroys = pool.get_thread_destroy_count();
        double avg_thread_lifetime = thread_destroys > 0 ? (double)pool.get_total_thread_lifetime_ms() / thread_destroys : 0;

        std::cout << "--- Adaptive-A Pool Metrics ---" << std::endl;
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
        std::cout << "-------------------------------" << std::endl;
    }

    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> " << "Adaptive-A Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 3: C++ 기반 Adaptive-B (Queue Waiting Time 기반)
    std::cout << "\n[" << "Adaptive-B Mode (2, 10 threads)" << " 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    std::cout << "설정 : Min(" << 2 << "), Max(" << 10 << ")" << std::endl;

    start = std::chrono::system_clock::now();

    {
        AdaptiveThreadPoolB pool(2, 10);

        for (int i = 0; i < total_tasks; i++) {
            pool.submit([i]() { io_workload(i); });
        }
        
        // 잠시 대기하여 작업 완료 및 스레드 축소(scale-down) 유도 (B는 timeout 5초)
        std::this_thread::sleep_for(std::chrono::seconds(7));

        size_t completed = pool.get_completed_task_count();
        double avg_latency = completed > 0 ? (double)pool.get_total_latency_ms() / completed : 0;
        double avg_queue_wait = completed > 0 ? (double)pool.get_total_queue_wait_time_ms() / completed : 0;
        double avg_service = completed > 0 ? (double)pool.get_total_service_time_ms() / completed : 0;
        size_t thread_destroys = pool.get_thread_destroy_count();
        double avg_thread_lifetime = thread_destroys > 0 ? (double)pool.get_total_thread_lifetime_ms() / thread_destroys : 0;

        std::cout << "--- Adaptive-B Pool Metrics ---" << std::endl;
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
        std::cout << "-------------------------------" << std::endl;
    }

    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> " << "Adaptive-B Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    // 실험 4: C++ 기반 Adaptive-C (Stability/Duration 기반)
    std::cout << "\n[" << "Adaptive-C Mode (2, 10 threads)" << " 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    std::cout << "설정 : Min(" << 2 << "), Max(" << 10 << ")" << std::endl;

    start = std::chrono::system_clock::now();

    {
        AdaptiveThreadPoolC pool(2, 10);

        for (int i = 0; i < total_tasks; i++) {
            pool.submit([i]() { io_workload(i); });
        }
        
        // 잠시 대기하여 작업 완료 및 스레드 축소(scale-down) 유도 (C는 timeout 10초)
        std::this_thread::sleep_for(std::chrono::seconds(12));

        size_t completed = pool.get_completed_task_count();
        double avg_latency = completed > 0 ? (double)pool.get_total_latency_ms() / completed : 0;
        double avg_queue_wait = completed > 0 ? (double)pool.get_total_queue_wait_time_ms() / completed : 0;
        double avg_service = completed > 0 ? (double)pool.get_total_service_time_ms() / completed : 0;
        size_t thread_destroys = pool.get_thread_destroy_count();
        double avg_thread_lifetime = thread_destroys > 0 ? (double)pool.get_total_thread_lifetime_ms() / thread_destroys : 0;

        std::cout << "--- Adaptive-C Pool Metrics ---" << std::endl;
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
        std::cout << "-------------------------------" << std::endl;
    }

    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << ">>> " << "Adaptive-C Mode 총 소요 시간: " << diff.count() << "s" << std::endl;
    std::cout << ">>> Throughput (tasks/s): " << (total_tasks / diff.count()) << std::endl;

    return 0;
}