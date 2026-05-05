# 스레드 풀 성능 비교 연구

이 프로젝트는 다양한 워크로드 특성에서 고정(Fixed) 스레드 풀과 여러 적응형(Adaptive) 스레드 풀 구현의 성능, 안정성 및 복원력을 비교하는 것을 목표로 합니다.

## 프로젝트 구조

*   `include/`: `pthread_pool`, `adaptive_thread_pool`, `adaptive_thread_pool_b`, `adaptive_thread_pool_c` 헤더 파일.
*   `src/`: 스레드 풀 구현을 위한 소스 파일.
*   `tests/`: 다양한 워크로드를 위한 테스트 시나리오.
*   `main.cpp`: (테스트에는 직접 사용되지 않지만 다른 목적으로 활용 가능)

## 스레드 풀 구현체

비교를 위해 세 가지 적응형 스레드 풀 정책이 구현되었습니다:

1.  **Adaptive-A (큐 길이 기반)**:
    *   **스케일업 (Scale-up)**: 큐 길이가 활성 스레드 수의 두 배를 초과하면 새 스레드를 생성합니다.
    *   **스케일다운 (Scale-down)**: 스레드가 5초 동안 유휴 상태인 경우 해당 스레드를 소멸시킵니다.
2.  **Adaptive-B (큐 대기 시간 기반)**:
    *   **스케일업 (Scale-up)**: 평균 큐 대기 시간을 모니터링합니다. 50ms를 초과하면 새 스레드를 생성합니다.
    *   **스케일다운 (Scale-down)**: 스레드가 5초 동안 유휴 상태인 경우 해당 스레드를 소멸시킵니다.
3.  **Adaptive-C (안정성/지속 시간 기반)**:
    *   **스케일업 (Scale-up)**: 큐가 긴 상태(큐 길이 > 활성 스레드 수 * 2)가 3초 이상 지속될 경우에만 새 스레드를 생성하여, 급격한 스케일링보다는 안정성을 목표로 합니다.
    *   **스케일다운 (Scale-down)**: 스레드가 10초 동안 유휴 상태인 경우 해당 스레드를 소멸시켜, 스레드 수명을 길게 유지합니다.

## 수집된 성능 메트릭

각 테스트 시나리오는 고정 및 적응형 스레드 풀 모두에 대해 다음 메트릭을 수집하고 보고합니다:

*   **Total Execution Time (총 실행 시간)**: 모든 작업을 완료하는 데 걸린 전체 시간.
*   **Throughput (처리량)**: 초당 처리된 작업 수.
*   **Max Thread Count (최대 스레드 수)**: 테스트 중 동시에 활성화된 최대 스레드 수.
*   **Thread Create Count (스레드 생성 횟수)**: 생성된 총 스레드 수.
*   **Thread Destroy Count (스레드 소멸 횟수)**: 소멸된 총 스레드 수.
*   **Resize Count (크기 조정 횟수)**: 스레드 풀 크기 조정(생성 + 소멸)의 총 횟수.
*   **Total Idle Time (ms) (총 유휴 시간)**: 모든 워커 스레드가 유휴 상태로 보낸 누적 시간 (밀리초).
*   **Completed Tasks (완료된 작업 수)**: 성공적으로 처리된 총 작업 수.
*   **Avg Latency (ms) (평균 지연 시간)**: 작업 제출부터 완료까지의 평균 시간 (밀리초).
*   **Avg Queue Wait (ms) (평균 큐 대기 시간)**: 작업이 워커에게 선택되기 전까지 큐에서 대기한 평균 시간 (밀리초).
*   **Avg Service Time (ms) (평균 서비스 시간)**: 워커가 작업을 적극적으로 처리하는 데 보낸 평균 시간 (밀리초).
*   **Avg Thread Lifetime (ms) (평균 스레드 수명)**: 소멸된 스레드의 평균 수명 (밀리초).

## 테스트 시나리오 및 실행 방법

각 테스트를 컴파일하고 실행하려면 먼저 프로젝트를 GitHub에서 클론(clone)한 후 다음 단계를 따르십시오.

1.  **프로젝트 클론**:
    ```bash
    git clone [Your-Repo-URL] # 여기에 실제 GitHub 저장소 URL을 입력하세요.
    cd [Your-Repo-Name] # 클론된 저장소 디렉토리 이름
    ```

2.  **필요한 도구 설치 (Ubuntu/Debian 기반 시스템)**:
    `g++` 컴파일러가 설치되어 있지 않다면 다음 명령어로 설치합니다:
    ```bash
    sudo apt update
    sudo apt install build-essential
    ```

3.  **각 테스트 컴파일 및 실행**:
    각 테스트 시나리오는 웜업(warm-up) 30개 태스크를 먼저 수행한 후, 주요 태스크 10,000개를 실행하도록 구성되어 있으며, 결과는 `test_results.csv` 파일에 자동으로 추가됩니다.

    ### 🌟 자동화된 전체 테스트 실행 (권장)
    각 테스트를 개별적으로 컴파일하고 실행하는 번거로움을 줄이기 위해, 모든 테스트를 자동으로 컴파일하고 **각각 30번씩 반복 실행**하는 스크립트를 제공합니다. 충분한 데이터 포인트 수집을 위해 아래 명령어를 사용하는 것을 권장합니다:
    ```bash
    ./run_all_tests.sh
    ```

    *(아래는 개별적으로 테스트를 실행하고자 할 때의 명령어입니다)*

    *   **1. CPU Burst 워크로드**
        *   **파일**: `tests/test_cpu_bound.cpp`
        *   **목적**: 주로 CPU 사이클을 소비하는 무거운 계산 작업 부하에서 스레드 풀 성능을 평가합니다.
        *   **명령어**:
            ```bash
            g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_cpu_bound.cpp -o test_cpu && ./test_cpu
            ```
    *   **2. I/O Burst 워크로드**
        *   **파일**: `tests/test_io_bound.cpp`
        *   **목적**: 스레드 풀이 대기 시간이 긴 작업(예: 디스크 I/O, 네트워크 요청 시뮬레이션)을 얼마나 잘 처리하는지 평가합니다.
        *   **명령어**:
            ```bash
            g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_io_bound.cpp -o test_io && ./test_io
            ```
    *   **3. Mixed 워크로드**
        *   **파일**: `tests/test_mixed_bound.cpp`
        *   **목적**: 계산 및 I/O 작업이 모두 포함된 보다 현실적인 시나리오를 시뮬레이션합니다.
        *   **명령어**:
            ```bash
            g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_mixed_bound.cpp -o test_mixed && ./test_mixed
            ```
    *   **4. Burst Traffic 워크로드**
        *   **파일**: `tests/test_burst_bound.cpp`
        *   **목적**: 작업 제출의 갑작스럽고 간헐적인 스파이크에 스레드 풀이 얼마나 잘 반응하는지 테스트합니다.
        *   **명령어**:
            ```bash
            g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_burst_bound.cpp -o test_burst && ./test_burst
            ```
    *   **5. Spike + Recovery 워크로드**
        *   **파일**: `tests/test_spike_bound.cpp`
        *   **목적**: 강렬하고 짧은 기간의 작업 급증 시 스레드 풀의 성능과 이후 효율적으로 스케일다운하고 복구하는 능력을 측정합니다.
        *   **명령어**:
            ```bash
            g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_spike_bound.cpp -o test_spike && ./test_spike
            ```
    *   **6. 안정성 중심 워크로드**
        *   **파일**: `tests/test_stability_bound.cpp`
        *   **목적**: 지속적이고 변동하는 부하(마이크로 버스트 포함)에서 과도한 스레드 생성/소멸을 피하면서 다양한 적응형 정책이 안정성을 얼마나 잘 유지하는지 평가합니다.
        *   **명령어**:
            ```bash
            g++ -std=c++11 -pthread -Iinclude src/*.cpp tests/test_stability_bound.cpp -o test_stability && ./test_stability
            ```