#!/usr/bin/env bash
set -euo pipefail

./beckon/tests/check-keymap.sh
./beckon/tests/test-status-protocol.sh
./beckon/tests/check-memory.sh beckon/tests/fixtures/memory-ok.txt
./beckon/tests/test-install-candidate-macos.sh
