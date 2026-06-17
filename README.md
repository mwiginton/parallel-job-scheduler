# Parallel Job Scheduler

A terminal-based C++ job scheduler project.

The current version reads shell commands and dependencies from a JSON file, topologically sorts the jobs, and runs them sequentially through a small `JobRunner`.

## Build

```powershell
C:\msys64\ucrt64\bin\cmake.exe -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=C:\msys64\ucrt64\bin\g++.exe
C:\msys64\ucrt64\bin\cmake.exe --build build
```

## Run

```powershell
.\build\parallel_job_scheduler.exe
```

You can also pass a config path explicitly:

```powershell
.\build\parallel_job_scheduler.exe jobs.json
```

Expected output:

```text
Running job: prepare
Command: echo Preparing job inputs
Preparing job inputs
Job exited with code: 0
Running job: build
Command: echo Building project artifacts
Building project artifacts
Job exited with code: 0
Running job: test
Command: echo Running tests
Running tests
Job exited with code: 0
Running job: package
Command: echo Packaging final output
Packaging final output
Job exited with code: 0
All jobs completed successfully.
```

## Config Format

```json
{
  "jobs": [
    {
      "name": "package",
      "command": "echo Packaging final output",
      "dependencies": ["build", "test"]
    },
    {
      "name": "test",
      "command": "echo Running tests",
      "dependencies": ["build"]
    },
    {
      "name": "build",
      "command": "echo Building project artifacts",
      "dependencies": ["prepare"]
    },
    {
      "name": "prepare",
      "command": "echo Preparing job inputs"
    }
  ]
}
```

The `dependencies` field is optional. If present, every dependency must match another job's `name`.

## Test

```powershell
C:\msys64\ucrt64\bin\ctest.exe --test-dir build --output-on-failure
```
