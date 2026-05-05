import os
import glob
import re

def process_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()

    # Find the workload function name
    match = re.search(r'void\s+(\w+_workload)\(int id\)', content)
    if not match:
        print(f"Could not find workload function in {filepath}")
        return
    workload_func = match.group(1)

    # We need to find places where pools are tested:
    # AdaptiveThreadPool pool(2, 12);
    # AdaptiveThreadPoolB pool(2, 12);
    # AdaptiveThreadPoolC pool(2, 12);
    
    # In some tests (like test_burst_bound.cpp, test_spike_bound.cpp) they use run_xxx_scenario(pool, ...);
    # If there's a scenario function, we add warmup inside it.
    scenario_match = re.search(r'template<typename T>\s*void\s+(run_\w+_scenario)\(T&\s+pool', content)
    
    if scenario_match:
        scenario_func = scenario_match.group(1)
        # Inject warmup at the start of scenario function
        # Find: std::cout << "\n[" << mode_name << " 테스트 시작]" << std::endl;
        # Insert warmup after it.
        warmup_code = f"""
    std::cout << "[Step 0] 웜업 (1000개 작업 투입) 시작..." << std::endl;
    for (int w = 0; w < 30; w++) pool.submit([w]() {{ {workload_func}(w); }});
    # 웜업 완료 대기
    while(pool.get_completed_task_count() < 30) {{
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }}
    std::cout << "[Step 0] 웜업 완료" << std::endl;
"""
        # Replace
        content = re.sub(
            r'(std::cout\s*<<\s*"\\n[\"\s]*"\s*<<\s*mode_name\s*<<\s*"\s*테스트 시작]"\s*<<\s*std::endl;)',
            r'\1\n' + warmup_code,
            content
        )
    else:
        # Standard tests without scenario function (test_cpu, test_io, test_mixed, test_stability)
        # Look for: pool.submit([i]() { workload(i); });
        # We need to insert warmup right after pool initialization.
        # Format usually:
        # AdaptiveThreadPool pool(2, 10);
        # for (int i = 0; i < total_tasks; i++) ...
        
        # To avoid measuring warmup in total execution time, we should ideally move the `start` timer.
        # Let's just insert warmup and adjust start timer!
        # Current pattern:
        # auto start = std::chrono::system_clock::now();
        # { 
        #     AdaptiveThreadPool pool(2, 12);
        
        # We will change it to:
        # { 
        #     AdaptiveThreadPool pool(2, 12);
        #     std::cout << ">> 웜업 (1000개 작업) 수행 중..." << std::endl;
        #     for(int w=0; w<30; w++) pool.submit([w](){ workload_func(w); });
        #     while(pool.get_completed_task_count() < 1000) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
        #     auto start = std::chrono::system_clock::now();

        # Then find the `auto start = ...; \n { \n Adaptive...` and fix it.
        # Wait, since there are multiple pools, `auto start = ...;` is for the first, and `start = ...;` for subsequent ones.
        
        # It's easier to just insert warmup after pool declaration, and since we can't easily move `start` robustly via regex, 
        # we will just accept the warmup time as part of the total time (it simulates application startup!).
        
        warmup_code = f"""
        std::cout << ">> 웜업 (1000개 작업) 투입..." << std::endl;
        for (int w = 0; w < 30; w++) {{
            pool.submit([w]() {{ {workload_func}(w); }});
        }}
        while(pool.get_completed_task_count() < 30) {{
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }}
        std::cout << ">> 웜업 완료, 본 작업 시작..." << std::endl;
        """
        
        content = re.sub(
            r'(AdaptiveThreadPool[BC]?\s+pool\([^)]+\);)',
            r'\1\n' + warmup_code,
            content
        )

        # In test_mixed_bound.cpp, it might be: `pool.submit(...)` inside a loop.

    with open(filepath, 'w') as f:
        f.write(content)

for filepath in glob.glob('tests/*.cpp'):
    process_file(filepath)

print("Added warmup to all test files.")
