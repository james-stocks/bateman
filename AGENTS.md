# AGENTS.md

## Cursor Cloud specific instructions

### Product overview

**Bateman** is a single LLVM-based compiler (C++/Flex/Bison). There is no monorepo, no `package.json`, and no long-running dev server. End-to-end validation means: build `build/bateman` → compile a `.bateman` file → run the resulting native binary.

### System dependencies (Ubuntu)

Match `.github/workflows/build-and-test.yaml`. One-time install on a fresh VM:

```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential flex bison llvm-18-dev zlib1g-dev clang-18 libfl-dev libzstd-dev libstdc++-14-dev
```

**Gotcha:** Default `c++` may be Clang 18, which links against GCC 14’s `libstdc++`. If CMake reports `cannot find -lstdc++`, install `libstdc++-14-dev`. If CMake reports missing `zstd::libzstd_shared`, install `libzstd-dev`.

### Build / run (standard commands)

Configure (first time or after `make clean`):

```bash
cmake -S . -B build -DCMAKE_CXX_FLAGS="-I/usr/include" -DLLVM_DIR=/usr/lib/llvm-18/cmake
```

Build (also available via `make` / `make rebuild`):

```bash
cmake --build build --config Release
# or: make
```

Compile and run a program (CI smoke test):

```bash
build/bateman test/dorsia.bateman output_binary
./output_binary
```

Compiler CLI: `bateman [source.bateman] [output_executable_name]` (defaults: `input.bateman`, `resulting_executable`).

### Lint / tests

There is no project-local linter or unit-test harness. **CI is the test suite** (`.github/workflows/build-and-test.yaml`). Re-run the dorsia compile + execute steps above to validate changes.

### Services

No databases, Docker Compose, or HTTP services. Only the build toolchain and produced binaries.

### After `git pull`

If compiler sources under `src/` or `CMakeLists.txt` changed, rebuild with `make` or `cmake --build build`. The VM update script does not run builds automatically (see SetupVmEnvironment).

### VS Code extension (`vscode/bateman-syntax/`)

When you change `src/lexer.l` or `src/parser.y`, update the syntax extension in the same change:

- `vscode/bateman-syntax/syntaxes/bateman.tmLanguage.json`
- `vscode/bateman-syntax/language-configuration.json`
