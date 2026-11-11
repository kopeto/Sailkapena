#!/usr/bin/env bash
set -euo pipefail

PKGS=(build-essential cmake qt6-base-dev)

if ! command -v apt-get >/dev/null 2>&1; then
  echo "Script honek Debian edo Ubuntu eta apt-get behar ditu." >&2
  exit 1
fi

SUDO=()
if [[ "$(id -u)" -ne 0 ]]; then
  if command -v sudo >/dev/null 2>&1; then
    SUDO=(sudo)
  else
    echo "Instalatu sudo edo exekutatu script hau root erabiltzailearekin." >&2
    exit 1
  fi
fi

missing=()
for package in "${PKGS[@]}"; do
  if ! dpkg -s "$package" >/dev/null 2>&1; then
    missing+=("$package")
  fi
done

if ((${#missing[@]})); then
  "${SUDO[@]}" apt-get update
  "${SUDO[@]}" apt-get install -y "${missing[@]}"
fi

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel "$(nproc)"
