# 🚀 Execution Guide (macOS)

본 프로젝트는 **macOS (Apple Silicon 및 Intel)** 환경에서 `g++` (Clang wrapper)를 사용하여 빌드 및 실행할 수 있도록 설계되었습니다.
C++17 표준과 POSIX 스레드 라이브러리(`pthread`)를 사용하여 구현되었습니다.

---

### 1. 전제 조건 (Prerequisites)

맥북에 **Command Line Tools**가 설치되어 있어야 합니다. 설치되지 않았다면 터미널에서 아래 명령어를 실행하여 설치하세요.

```bash
xcode-select --install
```

### 2. 컴파일 (Compile)

터미널을 열고 프로젝트 루트 폴더(SW-AI-Convergence-R-D)로 이동한 뒤, 아래 명령어를 입력하여 실행 파일(threadpool_test)을 생성합니다.

```bash
g++ -std=c++17 -Wall -o threadpool_test ThreadPool.cpp main.cpp -lpthread
```

-std=c++17: C++17 표준 문법을 사용합니다.

-Wall: 모든 컴파일 경고(Warning)를 표시하여 코드의 잠재적 문제를 확인합니다.

-o threadpool_test: 생성될 실행 파일의 이름을 지정합니다.

-lpthread: [중요] POSIX 스레드 라이브러리를 명시적으로 링크합니다.

### 3. 실행 (Run)

빌드가 성공적으로 완료되면 아래 명령어로 프로그램을 실행합니다.

```bash
./threadpool_test
```

💡 주요 구현 특징 (Key Features)
Implicit Interface: 기존 C 스타일의 void\* 인자 전달 방식 대신, **C++ 람다(Lambda)**를 활용하여 사용자가 실행할 코드 블록을 직접 제출할 수 있도록 추상화하였습니다.

RAII Pattern: 객체의 생성자에서 스레드를 생성하고, 소멸자에서 shutdown 및 자원 해제를 자동으로 수행하도록 설계하여 안정성을 높였습니다.

POSIX Thread Control: 인터페이스는 현대적이지만, 내부는 pthread_mutex와 pthread_cond를 직접 제어하여 정밀한 동기화를 보장합니다.

⚠️ 문제 해결 (Troubleshooting)
'pthread.h' file not found: xcode-select --install 명령어를 통해 개발 도구가 정상적으로 설치되었는지 다시 확인하십시오.

Permission Denied: 실행 권한이 없는 경우 chmod +x threadpool_test 명령어로 권한을 부여한 뒤 실행하십시오.
