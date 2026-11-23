# Outdoor student starter

Implement your renderer in `src/StudentRenderer.cpp` and additional student-owned source/shader files. The supplied starter renders terrain and the camera frustum only. Complete the outdoor requirements in `../spec`.

The framework supplies cameras, terrain, firefly transforms and frustum testing, HDR output, and final Reinhard tone mapping with gamma correction. The student executable has no profiling/replay mode or benchmark report generation. Interactive FPS and frame-time display remain available.

Build from this directory:

```powershell
cmake -S builder -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build --config Release
```

Run the interactive application without arguments:

```powershell
.\build\Release\CG2026.exe
```

Run with `--player_only` to fill the window with the player view:

```powershell
.\build\Release\CG2026.exe --player_only
```

For a hidden 19-frame startup/resize smoke run:

```powershell
.\build\Release\CG2026.exe --smoke-test
```

Add `--player_only` to the smoke command to exercise the full-window layout. The starter smoke-check hook always returns true, so this run checks startup and resize only.

The separate instructor project, `../final_outdoor`, retains profiling support. This student framework intentionally differs from the instructor protected-file manifest. Running `../scripts/benchmark.py --official` on this directory replaces protected framework files with instructor copies and restores profiling support. Keep the student handout separate from that grading operation.
