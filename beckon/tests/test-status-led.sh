#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/../.." && pwd)
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/beckon-status-led.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror \
  -DCONFIG_BECKON_STATUS_LED_BRIGHTNESS_PERCENT=80 \
  -I "$root/beckon/module/include" \
  "$root/beckon/module/src/status_led_color.c" \
  "$root/beckon/tests/test-status-led.c" \
  -o "$work_dir/test-status-led"
"$work_dir/test-status-led"
