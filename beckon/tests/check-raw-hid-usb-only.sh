#!/usr/bin/env bash
set -euo pipefail

build_dir="${1:?usage: check-raw-hid-usb-only.sh BUILD_DIR}"

if find "$build_dir" -path '*zmk-raw-hid*hog.c.obj' -print -quit | grep -q .; then
  echo "Raw HID Bluetooth service was built; Beckon v1 must remain USB-only" >&2
  exit 1
fi

grep -Fq 'CONFIG_RAW_HID=y' "$build_dir/zephyr/.config"
grep -Fq '# CONFIG_RAW_HID_BLE is not set' "$build_dir/zephyr/.config"
echo "Raw HID USB-only invariant passed"
