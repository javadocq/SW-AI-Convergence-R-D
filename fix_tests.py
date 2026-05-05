import glob

def process_file(filepath):
    with open(filepath, 'r') as f:
        lines = f.readlines()

    new_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        if 'Total Execution Time (s)' in line:
            line = line.replace('Total Execution Time (s)', 'Total Execution Time (ms)')
        if 'double total_execution_time_s' in line:
            line = line.replace('double total_execution_time_s', 'double total_execution_time_ms')
        if 'total_execution_time_s' in line and 'save_metrics_to_csv' not in line: # Fix parameter name in call
            line = line.replace('total_execution_time_s', 'total_execution_time_ms')
            
        if 'auto diff = end - start;' in line:
            new_lines.append("        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);\n")
            i += 1
            # next line should be double var_exec_time = diff.count();
            exec_time_line = lines[i]
            var_name = exec_time_line.strip().split()[1].replace('_exec_time', '')
            new_lines.append(exec_time_line)
            i += 1
            # next line should be double var_throughput = ...
            throughput_line = f"        double {var_name}_throughput = {var_name}_exec_time > 0 ? (total_tasks / ({var_name}_exec_time / 1000.0)) : 0;\n"
            new_lines.append(throughput_line)
            i += 1
            continue
            
        new_lines.append(line)
        i += 1

    with open(filepath, 'w') as f:
        f.writelines(new_lines)

for filepath in glob.glob('tests/*.cpp'):
    process_file(filepath)

print("Processed all test files.")
