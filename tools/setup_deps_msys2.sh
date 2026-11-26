#!/usr/bin/env bash
# Downloads and builds the dependencies of the MMCE Unofficial Rebuild with MSYS2 (clang64).
#
#   1. Install MSYS2 (https://www.msys2.org)
#   2. Open the "MSYS2 CLANG64" shell, cd into this repository
#   3. ./tools/setup_deps_msys2.sh
#   4. cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build
#
# Everything is placed in ./deps (ignored by git).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEPS="$ROOT/deps"
mkdir -p "$DEPS"
cd "$DEPS"

echo "== MSYS2 packages =="
pacman -S --needed --noconfirm \
  mingw-w64-clang-x86_64-clang mingw-w64-clang-x86_64-cmake mingw-w64-clang-x86_64-ninja \
  mingw-w64-clang-x86_64-glew mingw-w64-clang-x86_64-eigen3 mingw-w64-clang-x86_64-glm \
  mingw-w64-clang-x86_64-freetype mingw-w64-clang-x86_64-flac mingw-w64-clang-x86_64-libvorbis \
  mingw-w64-clang-x86_64-libogg mingw-w64-clang-x86_64-openal unzip curl git

echo "== SFML 2.6.2 =="
# The game needs SFML 2.x (MSYS2 ships SFML 3). libc++ >= 19 removed the generic
# std::char_traits, which SFML 2 needs => small compatibility header (third_party/).
if [ ! -d SFML-src ]; then
  git clone --depth 1 --branch 2.6.2 https://github.com/SFML/SFML.git SFML-src
fi
cp "$ROOT/third_party/CharTraitsShim.hpp" SFML-src/include/SFML/CharTraitsShim.hpp
if ! grep -q CharTraitsShim SFML-src/include/SFML/Config.hpp; then
  sed -i 's|#define SFML_CONFIG_HPP|#define SFML_CONFIG_HPP\n\n// MMCE rebuild: libc++ >= 19 compatibility\n#include <SFML/CharTraitsShim.hpp>|' \
    SFML-src/include/SFML/Config.hpp
fi
cmake -S SFML-src -B SFML-build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
  -DSFML_USE_SYSTEM_DEPS=ON -DSFML_BUILD_NETWORK=OFF -DCMAKE_INSTALL_PREFIX="$DEPS/SFML" \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build SFML-build
cmake --install SFML-build

echo "== AntTweakBar 1.16 =="
if [ ! -d atb/AntTweakBar ]; then
  mkdir -p atb
  curl -L -o AntTweakBar_116.zip "https://sourceforge.net/projects/anttweakbar/files/AntTweakBar_116.zip/download"
  unzip -q -o AntTweakBar_116.zip -d atb
fi
cd atb/AntTweakBar/src
# patches: the Direct3D 11 backend needs precompiled shader headers (not shipped) and is
# not used by the game (OpenGL only); MAKEINTRESOURCE on cursor ids does not compile with clang
sed -i 's/^#   include "TwDirect3D11.h"/\/\/ (MMCE rebuild: Direct3D 11 backend disabled, the game uses OpenGL)/; s/g_TwMgr->m_Graph = new CTwGraphDirect3D11;/g_TwMgr->m_Graph = NULL; \/\/ Direct3D 11 disabled/; s/MAKEINTRESOURCE(IDC_\([A-Z]*\))/IDC_\1/g' TwMgr.cpp
mkdir -p obj
for f in TwMgr.cpp TwBar.cpp TwColors.cpp TwFonts.cpp TwOpenGL.cpp TwOpenGLCore.cpp TwPrecomp.cpp LoadOGL.cpp LoadOGLCore.cpp TwDirect3D9.cpp TwDirect3D10.cpp; do
  clang++ -std=c++14 -O2 -DTW_STATIC -DTW_NO_LIB_PRAGMA -D_WINDOWS -DWIN32 \
    -DMCW_PC=0x00030000 -D_PC_24=0x00020000 -D_PC_53=0x00010000 -I../include -w -c "$f" -o "obj/${f%.*}.o"
done
llvm-ar rcs ../lib/libAntTweakBar.a obj/*.o

echo
echo "Done. Now build the game:"
echo "  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build"
echo "  ./tools/package_windows.sh   (copies the DLLs next to build/MarbleMarcher.exe)"
