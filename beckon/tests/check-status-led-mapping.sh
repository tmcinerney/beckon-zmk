#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/../.." && pwd)
keymap="$root/beckon/config/beckon.keymap"
board="$root/app/boards/arm/glove80/glove80_lh.dts"
header="$root/beckon/module/include/beckon/status_led.h"

f1_position=$(awk '/^#define POS_LH_C6R1 / { print $3; exit }' "$keymap")
if [[ "$f1_position" != "0" ]]; then
  echo "expected Glove80 F1 matrix position 0, found ${f1_position:-missing}" >&2
  exit 1
fi

mapfile -t pixels < <(
  sed -n '/pixel-lookup =/,/;/p' "$board" | grep -oE '<[0-9]+>' | tr -d '<>'
)
for index in "${!pixels[@]}"; do
  if [[ "${pixels[$index]}" == "$f1_position" ]]; then
    f1_pixel="$index"
    break
  fi
done

expected=$(awk '/^#define BECKON_GLOVE80_LEFT_F1_PIXEL / { print $3; exit }' "$header")
if [[ -z "${f1_pixel:-}" || "$expected" != "$f1_pixel" ]]; then
  echo "Beckon F1 LED mapping drifted: expected ${expected:-missing}, DTS maps F1 to ${f1_pixel:-missing}" >&2
  exit 1
fi

echo "Beckon F1 LED mapping passed: matrix position $f1_position -> pixel $f1_pixel"
