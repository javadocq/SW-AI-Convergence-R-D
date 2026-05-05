import os
import glob
import re

# Update thread pool source files
def replace_in_file(filepath, pattern, replacement):
    with open(filepath, 'r') as f:
        content = f.read()
    content = re.sub(pattern, replacement, content)
    with open(filepath, 'w') as f:
        f.write(content)

replace_in_file('src/adaptive_thread_pool.cpp', r'std::chrono::seconds\(5\)', 'std::chrono::seconds(1)')
replace_in_file('src/adaptive_thread_pool_b.cpp', r'std::chrono::seconds\(5\)', 'std::chrono::seconds(1)')
replace_in_file('src/adaptive_thread_pool_c.cpp', r'std::chrono::seconds\(10\)', 'std::chrono::seconds(2)')

# Update test files
for filepath in glob.glob('tests/*.cpp'):
    with open(filepath, 'r') as f:
        content = f.read()
    
    content = re.sub(r'std::this_thread::sleep_for\(std::chrono::seconds\(7\)\);', 'std::this_thread::sleep_for(std::chrono::seconds(2));', content)
    content = re.sub(r'std::this_thread::sleep_for\(std::chrono::seconds\(12\)\);', 'std::this_thread::sleep_for(std::chrono::seconds(3));', content)
    content = re.sub(r'std::this_thread::sleep_for\(std::chrono::seconds\(13\)\);', 'std::this_thread::sleep_for(std::chrono::seconds(3));', content)
    
    with open(filepath, 'w') as f:
        f.write(content)

print("Wait times updated successfully.")
