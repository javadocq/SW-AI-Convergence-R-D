#include "../include/adaptive_thread_pool.hpp"
#include "../include/adaptive_thread_pool_b.hpp"
#include "../include/adaptive_thread_pool_c.hpp"
#include "../include/pthread_pool.hpp" 
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <fstream> // Added for file operations
#include <iomanip> // Added for std::fixed and std::setprecision

// New function to save metrics to CSV
void save_metrics_to_csv(
    const std::string& test_scenario,
    const std::string& pool_type,
    double total_execution_time_ms,
    double throughput_tasks_per_s,
    size_t total_tasks_submitted,
    size_t max_thread_count,
    size_t thread_create_count,
    size_t thread_destroy_count,
    size_t resize_count,
    size_t total_idle_time_ms,
    size_t completed_tasks,
    double avg_latency_ms,
    double avg_queue_wait_ms,
    double avg_service_time_ms,
    double avg_thread_lifetime_ms
) {
    std::ofstream ofs("test_results.csv", std::ios::out | std::ios::app);
    if (!ofs.is_open()) {
        std::cerr << "Error: Could not open test_results.csv for writing." << std::endl;
        return;
    }

    // Write header if file is empty
    if (ofs.tellp() == 0) {
        ofs << "Test Scenario,Pool Type,Total Execution Time (ms),Throughput (tasks/s),Total Tasks Submitted,Max Thread Count,Thread Create Count,Thread Destroy Count,Resize Count,Total Idle Time (ms),Completed Tasks,Avg Latency (ms),Avg Queue Wait (ms),Avg Service Time (ms),Avg Thread Lifetime (ms)\n";
    }

    ofs << std::fixed << std::setprecision(3)
        << test_scenario << ","
        << pool_type << ","
        << total_execution_time_ms << ","
        << throughput_tasks_per_s << ","
        << total_tasks_submitted << ","
        << max_thread_count << ","
        << thread_create_count << ","
        << thread_destroy_count << ","
        << resize_count << ","
        << total_idle_time_ms << ","
        << completed_tasks << ","
        << avg_latency_ms << ","
        << avg_queue_wait_ms << ","
        << avg_service_time_ms << ","
        << avg_thread_lifetime_ms << "\n";

    ofs.close();
}

// 공통 워크로드: 0.5초 동안 대기 (I/O 시뮬레이션)
void io_workload(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main() {
    int total_tasks = 30; // 30개의 작업을 투척

    std::cout << "==========================================" << std::endl;
    std::cout << "   스레드 풀 성능 비교 실험 (I/O Bound)   " << std::endl; // Changed from Benchmark
    std::cout << "==========================================" << std::endl;

    // 실험 2: C++ 기반 Adaptive-A (Queue Length 기반)
    std::cout << "\n[" << "Adaptive-A Mode (2, 10 threads)" << " 테스트 시작] - 작업: " << total_tasks << "개" << std::endl;
    std::cout << "설정 : Min(" << 2 << "), Max(" << 10 << ")" << std::endl;

    auto start = std::chrono::system_clock::now();

    {
        AdaptiveThreadPool pool(2, 10); // Fixed typo: AdativeThreadPool -> AdaptiveThreadPool

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

        auto end = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double adaptive_a_exec_time = diff.count();
        double adaptive_a_throughput = adaptive_a_exec_time > 0 ? (total_tasks / (adaptive_a_exec_time / 1000.0)) : 0;
        std::cout << ">>> " << "Adaptive-A Mode 총 소요 시간: " << adaptive_a_exec_time << "s" << std::endl;
        std::cout << ">>> Throughput (tasks/s): " << adaptive_a_throughput << std::endl;

        save_metrics_to_csv(
            "I/O Bound", "Adaptive-A",
            adaptive_a_exec_time, adaptive_a_throughput, total_tasks,
            pool.get_max_thread_count(),
            pool.get_thread_create_count(),
            pool.get_thread_destroy_count(),
            pool.get_resize_count(),
            pool.get_total_idle_time_ms(),
            completed,
            avg_latency, avg_queue_wait, avg_service, avg_thread_lifetime
        );
    }

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

        auto end = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double adaptive_b_exec_time = diff.count();
        double adaptive_b_throughput = adaptive_b_exec_time > 0 ? (total_tasks / (adaptive_b_exec_time / 1000.0)) : 0;
        std::cout << ">>> " << "Adaptive-B Mode 총 소요 시간: " << adaptive_b_exec_time << "s" << std::endl;
        std::cout << ">>> Throughput (tasks/s): " << adaptive_b_throughput << std::endl;

        save_metrics_to_csv(
            "I/O Bound", "Adaptive-B",
            adaptive_b_exec_time, adaptive_b_throughput, total_tasks,
            pool.get_max_thread_count(),
            pool.get_thread_create_count(),
            pool.get_thread_destroy_count(),
            pool.get_resize_count(),
            pool.get_total_idle_time_ms(),
            completed,
            avg_latency, avg_queue_wait, avg_service, avg_thread_lifetime
        );
    }

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

        auto end = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double adaptive_c_exec_time = diff.count();
        double adaptive_c_throughput = adaptive_c_exec_time > 0 ? (total_tasks / (adaptive_c_exec_time / 1000.0)) : 0;
        std::cout << ">>> " << "Adaptive-C Mode 총 소요 시간: " << adaptive_c_exec_time << "s" << std::endl;
        std::cout << ">>> Throughput (tasks/s): " << adaptive_c_throughput << std::endl;

        save_metrics_to_csv(
            "I/O Bound", "Adaptive-C",
            adaptive_c_exec_time, adaptive_c_throughput, total_tasks,
            pool.get_max_thread_count(),
            pool.get_thread_create_count(),
            pool.get_thread_destroy_count(),
            pool.get_resize_count(),
            pool.get_total_idle_time_ms(),
            completed,
            avg_latency, avg_queue_wait, avg_service, avg_thread_lifetime
        );
    }

    return 0;
}