# CG2026

From the project root, configure and build the Release executable:

```powershell
cmake -S builder -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
cmake --build build --config Release --parallel
.\build\Release\CG2026.exe
```

With a single-configuration generator, configure with `-DCMAKE_BUILD_TYPE=Release` and run `./build/CG2026`.
