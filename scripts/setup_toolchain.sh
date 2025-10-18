#!/usr/bin/env bash
# Helper script to build a local i686-elf cross compiler (GCC + Binutils)
# Optional: use distro packages if available (e.g., pacman -S i686-elf-gcc)
# Minimal version; adapt as needed.
set -euo pipefail

PREFIX="$HOME/cross"
TARGET=i686-elf
BINUTILS_VER=2.43
GCC_VER=14.2.0

mkdir -p "$PREFIX/src" && cd "$PREFIX/src"

fetch() {
  local url="$1" name=$(basename "$1")
  if [ ! -f "$name" ]; then
    curl -LO "$url"
  fi
}

echo "[+] Fetching sources"
fetch https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VER}.tar.xz
fetch https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VER}/gcc-${GCC_VER}.tar.xz

if [ ! -d binutils-${BINUTILS_VER} ]; then tar xf binutils-${BINUTILS_VER}.tar.xz; fi
if [ ! -d gcc-${GCC_VER} ]; then tar xf gcc-${GCC_VER}.tar.xz; fi

mkdir -p build-binutils build-gcc

cd build-binutils
../binutils-${BINUTILS_VER}/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install
cd ..

echo "[+] Building GCC (Stage 1)" 
cd build-gcc
../gcc-${GCC_VER}/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc

cat <<EOF
============================================================
Cross toolchain installed. Add to PATH:
  export PATH="$PREFIX/bin:$PATH"
Then build: make all
============================================================
EOF
