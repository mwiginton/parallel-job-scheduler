# Parallel Job Scheduler

A terminal-based C++ job scheduler project.

The current version runs one hardcoded shell command through a small `JobRunner`.

## Build

```powershell
C:\msys64\ucrt64\bin\cmake.exe -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=C:\msys64\ucrt64\bin\g++.exe
C:\msys64\ucrt64\bin\cmake.exe --build build
```

## Run

```powershell
.\build\parallel_job_scheduler.exe
```

Expected output:

```text
Running job: hello-job
Command: echo Hello from the hardcoded job
Hello from the hardcoded job
Job exited with code: 0
```
