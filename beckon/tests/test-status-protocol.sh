#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/../.." && pwd)
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/beckon-status-protocol.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror \
  -I "$root/beckon/module/include" \
  "$root/beckon/module/src/protocol.c" \
  "$root/beckon/tests/test-status-protocol.c" \
  -o "$work_dir/test-status-protocol"
"$work_dir/test-status-protocol"
