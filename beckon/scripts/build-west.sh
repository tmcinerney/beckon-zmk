#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root"
python3 beckon/scripts/check-dependency-pins.py
if [[ ! -d .west ]]; then
  west init -l app
fi
west update --fetch-opt=--filter=tree:0
west zephyr-export
python3 beckon/scripts/check-dependency-pins.py --resolved

apply_patch() {
  local directory="$1" patch_file="$2"
  if git -C "$directory" apply --check "$patch_file"; then
    git -C "$directory" apply "$patch_file"
  else
    # AIDEV-NOTE: cached workspaces can already contain exactly this patch.
    git -C "$directory" apply --reverse --check "$patch_file"
  fi
}
apply_patch zmk-raw-hid "$root/beckon/patches/zmk-raw-hid-usb-only.patch"
apply_patch zephyr "$root/nix/input-report-no-block.patch"

for hand in lh rh; do
  conf="$root/beckon/config/beckon.conf"
  if [[ "$hand" == lh ]]; then
    conf+=";$root/beckon/config/transport.conf"
  fi
  west build -s app -d "build/beckon-$hand" -b "glove80_$hand" -- \
    -DKEYMAP_FILE="$root/beckon/config/beckon.keymap" \
    -DEXTRA_CONF_FILE="$conf" \
    -DZMK_EXTRA_MODULES="$root/beckon/module" 2>&1 | tee "beckon/$hand-build.log"
done
cat build/beckon-lh/zephyr/zmk.uf2 build/beckon-rh/zephyr/zmk.uf2 > beckon/glove80.uf2
cat beckon/lh-build.log beckon/rh-build.log > beckon/build.log
./beckon/tests/test-checks.sh
./beckon/tests/check-raw-hid-usb-only.sh build/beckon-lh
./beckon/tests/check-memory.sh beckon/build.log
west manifest --freeze > beckon/west-frozen.yml
printf '{"commit": "%s"}\n' "$(git rev-parse HEAD)" > beckon/source.json
