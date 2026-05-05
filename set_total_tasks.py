import glob
import re

def process_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()

    # Find and replace total_tasks variable
    # This pattern looks for 'int total_tasks = <number>;'
    # It also handles cases like: 'int total_tasks = 140; // 40 + 100'
    # Or in run_burst_scenario, the parameter total_tasks
    
    # Update total_tasks definition in main
    content = re.sub(
        r'(int\s+total_tasks\s*=\s*)(\d+)(\s*;/.*)?',
        r'\g<1>10000\g<3>',
        content
    )

    # For run_burst_scenario and run_spike_scenario, total_tasks is a parameter
    # and the actual tasks submitted are usually hardcoded (40+100 or 100+100).
    # I need to modify those specific loops as well.
    # Let's read these specific files and adjust.
    if "test_burst_bound.cpp" in filepath:
        # Step 2: 1차 Burst (40개 작업 투입) -> 5000개
        content = re.sub(
            r'(\s*for\s+\(int i = 0;\s+i < )(\d+)(;\s+i\+\+\) pool\.submit\(\[i\]\(\) \{ burst_workload\(i\); \}\);)',
            r'\g<1>5000\g<3>',
            content,
            count=1 # Only replace the first occurrence
        )
        # Step 4: 2차 Burst (100개 작업 투입) -> 5000개
        content = re.sub(
            r'(\s*for\s+\(int i = 0;\s+i < )(\d+)(;\s+i\+\+\) pool\.submit\(\[i\]\(\) \{ burst_workload\(i\); \}\);)',
            r'\g<1>5000\g<3>',
            content,
            count=1 # Only replace the first occurrence
        )
    elif "test_spike_bound.cpp" in filepath:
        # This one has a specific structure
        # I need to find the `run_spike_scenario` function
        # Inside `run_spike_scenario`, there are loops for 100 tasks, then 10 tasks
        
        # 1차 Spike (100개 작업 투입) -> 9000개
        content = re.sub(
            r'(\s*for\s+\(int i = 0;\s+i < )(\d+)(;\s+i\+\+\) pool\.submit\(\[i\]\(\) \{ spike_workload\(i\); \}\);)',
            r'\g<1>9000\g<3>',
            content,
            count=1 # Only replace the first occurrence
        )
        # 2차 소규모 부하 (10개 작업 투입) -> 1000개
        content = re.sub(
            r'(\s*for\s+\(int i = 0;\s+i < )(\d+)(;\s+i\+\+\) pool\.submit\(\[i\]\(\) \{ spike_workload\(i\); \}\);)',
            r'\g<1>1000\g<3>',
            content,
            count=1 # Only replace the first occurrence
        )


    with open(filepath, 'w') as f:
        f.write(content)

for filepath in glob.glob('tests/*.cpp'):
    process_file(filepath)

print("Updated total_tasks to 10000 in all test files.")
