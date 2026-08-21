#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/../.." && pwd)
keymap="$root/beckon/config/beckon.keymap"
header="$root/beckon/module/include/beckon/status_led.h"

assert_mapping() {
  local key_define=$1 board=$2 header_define=$3 expected_position=$4 label=$5
  local position pixel expected

  position=$(awk -v define="$key_define" '$2 == define { print $3; exit }' "$keymap")
  if [[ "$position" != "$expected_position" ]]; then
    echo "expected Glove80 $label matrix position $expected_position, found ${position:-missing}" >&2
    exit 1
  fi

  mapfile -t pixels < <(
    sed -n '/pixel-lookup =/,/;/p' "$board" | grep -oE '<[0-9]+>' | tr -d '<>'
  )
  for index in "${!pixels[@]}"; do
    if [[ "${pixels[$index]}" == "$position" ]]; then
      pixel="$index"
      break
    fi
  done

  expected=$(awk -v define="$header_define" '$2 == define { print $3; exit }' "$header")
  if [[ -z "${pixel:-}" || "$expected" != "$pixel" ]]; then
    echo "Beckon $label LED mapping drifted: expected ${expected:-missing}, DTS maps $label to ${pixel:-missing}" >&2
    exit 1
  fi

  echo "Beckon $label LED mapping passed: matrix position $position -> pixel $pixel"
}

assert_mapping POS_LH_C6R1 "$root/app/boards/arm/glove80/glove80_lh.dts" \
  BECKON_GLOVE80_LEFT_F1_PIXEL 0 F1
assert_mapping POS_RH_C2R1 "$root/app/boards/arm/glove80/glove80_rh.dts" \
  BECKON_GLOVE80_RIGHT_F6_PIXEL 5 F6
