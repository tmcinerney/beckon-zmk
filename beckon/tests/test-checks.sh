#!/usr/bin/env bash
set -euo pipefail

./beckon/tests/check-keymap.sh
./beckon/tests/check-memory.sh beckon/tests/fixtures/memory-ok.log
