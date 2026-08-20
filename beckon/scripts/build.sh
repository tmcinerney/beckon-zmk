#!/usr/bin/env bash
set -euo pipefail

if ! command -v nix-build >/dev/null 2>&1; then
  echo "nix-build is required. On macOS, run this inside the documented Docker builder." >&2
  exit 1
fi

rm -rf beckon/build
nix-build beckon/config -o beckon/build 2>&1 | tee beckon/build.log
./beckon/tests/check-keymap.sh
./beckon/tests/check-memory.sh beckon/build.log
echo "Built combined firmware: beckon/build/glove80.uf2"
