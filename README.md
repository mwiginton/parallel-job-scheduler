# Parallel Job Scheduler

A terminal-based C++ job scheduler project.

The current version reads shell commands and dependencies from a JSON file, validates the dependency graph, and runs independent jobs in parallel through a small thread pool.

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

The second argument sets the worker count:

```powershell
.\build\parallel_job_scheduler.exe jobs.json 2
```

Expected output:

```text
Scheduling 5 job(s) with 2 worker(s).
Running job: prepare
Command: echo Preparing job inputs
Preparing job inputs
Job exited with code: 0
Running job: build
Command: echo Building project artifacts
Building project artifacts
Job exited with code: 0
Running job: lint
Command: powershell -NoProfile -Command "Start-Sleep -Seconds 1; Write-Output 'Running lint checks'"
Running job: test
Command: powershell -NoProfile -Command "Start-Sleep -Seconds 1; Write-Output 'Running tests'"
Running tests
Running lint checks
Job exited with code: 0
Job exited with code: 0
Running job: package
Command: echo Packaging final output
Packaging final output
Job exited with code: 0
All jobs completed successfully.
```

The exact order of lines from concurrently running jobs may vary.

## Config Format

```json
{
  "jobs": [
    {
      "name": "package",
      "command": "echo Packaging final output",
      "dependencies": ["test", "lint"]
    },
    {
      "name": "lint",
      "command": "powershell -NoProfile -Command \"Start-Sleep -Seconds 1; Write-Output 'Running lint checks'\"",
      "dependencies": ["build"]
    },
    {
      "name": "test",
      "command": "powershell -NoProfile -Command \"Start-Sleep -Seconds 1; Write-Output 'Running tests'\"",
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

In the sample config, `test` and `lint` both depend on `build`, but neither depends on the other. Once `build` finishes, both become ready and can run at the same time.

## Test

```powershell
C:\msys64\ucrt64\bin\ctest.exe --test-dir build --output-on-failure
```
