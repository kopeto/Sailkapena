#!/usr/bin/env bash

./build.sh

targets=(HitzGurutzatuak)

for name in "${targets[@]}"; do
  src="build/$name"
  dst="/usr/local/bin/$name"

  if [[ -x "$src" ]]; then
    # -D: create leading dirs if needed
    # -m 0755: owner rwx, group rx, others rx
    sudo install -D -m 0755 "$src" "$dst"
    echo "Installed $name -> $dst"
  elif [[ -f "$src" ]]; then
    echo "Warning: $src exists but is not executable. Fixing perms and installing."
    sudo install -D -m 0755 "$src" "$dst"
  else
    echo "Skip: $src not found."
  fi
done