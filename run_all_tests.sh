#!/bin/bash

# 컴파일
echo "모든 테스트를 컴파일 중입니다..."
g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_cpu_bound.cpp -o test_cpu
g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_io_bound.cpp -o test_io
g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_mixed_bound.cpp -o test_mixed
g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_burst_bound.cpp -o test_burst
g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_spike_bound.cpp -o test_spike
g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_stability_bound.cpp -o test_stability

echo "컴파일 완료! 각 테스트를 30번씩 실행합니다..."

# 실행 횟수 설정
RUNS=3

for (( i=1; i<=RUNS; i++ ))
do
    echo "=========================================="
    echo " 실행 반복 횟수: $i / $RUNS"
    echo "=========================================="
    
    echo "실행 중: test_cpu"
    ./test_cpu
    
    echo "실행 중: test_io"
    ./test_io
    
    echo "실행 중: test_mixed"
    ./test_mixed
    
    echo "실행 중: test_burst"
    ./test_burst
    
    echo "실행 중: test_spike"
    ./test_spike
    
    echo "실행 중: test_stability"
    ./test_stability
done

echo "모든 테스트가 $RUNS 번씩 성공적으로 완료되었습니다!"
