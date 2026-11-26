#!/usr/bin/env bash
# Copies all DLLs the game needs next to build/MarbleMarcher.exe (MSYS2 clang64 build).
# Afterwards the build folder can be zipped and run on other Windows PCs.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build"
SFML_BIN="$ROOT/deps/SFML/bin"
[ -d "$SFML_BIN" ] || SFML_BIN="$ROOT/../deps/SFML/bin"

cd "$BUILD"
cp "$SFML_BIN"/*.dll .
export PATH="$SFML_BIN:/clang64/bin:/c/msys64/clang64/bin:$PATH"
# resolve the dependencies a few times (DLLs that need other DLLs)
for i in 1 2 3; do
  for f in *.exe *.dll; do
    ldd "$f" 2>/dev/null | grep -i "clang64" | awk '{print $3}'
  done | sort -u | while read -r d; do cp -n "$d" . ; done
done
echo "DLLs copied to $BUILD"
ldd MarbleMarcher.exe | grep -i "not found" || echo "all dependencies found"
