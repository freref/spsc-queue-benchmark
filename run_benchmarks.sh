#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

pushd "$ROOT/zig-spsc" >/dev/null
zig build --release=fast > /dev/null 2>&1

zig_exe="$(find zig-out/bin -maxdepth 1 -type f -perm -111 | head -n1)"
if [[ -z "${zig_exe:-}" ]]; then
  echo "Zig executable not found in zig-out/bin" >&2
  exit 1
fi
"${zig_exe}"
popd >/dev/null

pushd "$ROOT/cpp-spsc" >/dev/null
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. > /dev/null 2>&1
cmake --build . --config Release -- -j > /dev/null 2>&1

exe="./SPSCQueueBenchmark"
if [[ ! -x "$exe" ]]; then
  exe="$(find . -maxdepth 2 -type f -perm -111 -name 'SPSCQueueBenchmark*' | head -n1 || true)"
fi
[[ -n "${exe:-}" ]] || { echo "C++ benchmark executable not found" >&2; exit 1; }
"${exe}"
popd >/dev/null
