#!/usr/bin/env bash
set -euo pipefail

./build.sh

INSTALL_SUDO=()
if [[ "$(id -u)" -ne 0 ]]; then
  if command -v sudo >/dev/null 2>&1; then
    INSTALL_SUDO=(sudo)
  else
    echo "Instalatu sudo edo exekutatu script hau root erabiltzailearekin." >&2
    exit 1
  fi
fi

targets=(SailkapenaHG)

for name in "${targets[@]}"; do
  src="build/$name"
  dst="/usr/local/bin/$name"

  if [[ -x "$src" ]]; then
    # -D: create leading dirs if needed
    # -m 0755: owner rwx, group rx, others rx
    "${INSTALL_SUDO[@]}" install -D -m 0755 "$src" "$dst"
    echo "$name instalatuta: $dst"
  elif [[ -f "$src" ]]; then
    echo "Oharra: $src fitxategia ez da exekutagarria; baimenak zuzendu eta instalatuko da."
    "${INSTALL_SUDO[@]}" install -D -m 0755 "$src" "$dst"
  else
    echo "Ez da instalatu: ez da $src aurkitu."
  fi
done
