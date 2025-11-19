#!/bin/bash

# Packages to verify/install
PKGS=(
  libglu1-mesa-dev
  qt6-tools-dev
  qt6-base-dev
  qt6-base-dev-tools
)

# Require apt-based system
if ! command -v apt-get >/dev/null 2>&1; then
  echo "This script requires apt/apt-get (Debian/Ubuntu)." >&2
  exit 1
fi

# Use sudo if not root
SUDO=""
if [ "$(id -u)" -ne 0 ]; then
  if command -v sudo >/dev/null 2>&1; then
    SUDO="sudo"
  else
    echo "Please run as root or install sudo." >&2
    exit 1
  fi
fi

echo "Checking packages..."
missing=()
for p in "${PKGS[@]}"; do
  if dpkg -s "$p" >/dev/null 2>&1; then
    ver=$(dpkg-query -W -f='${Version}\n' "$p" 2>/dev/null || echo "unknown")
    echo "✓ $p is installed (version $ver)"
  else
    echo "✗ $p is NOT installed"
    missing+=("$p")
  fi
done

if [ "${#missing[@]}" -eq 0 ]; then
  echo "All packages are already installed."
else
    echo
    echo "Missing packages: ${missing[*]}"
    echo "Updating package lists..."
    $SUDO apt-get update -y

    echo "Installing missing packages..."
    $SUDO apt-get install -y "${missing[@]}"
    echo "Done."
fi
    
mkdir -p build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)

