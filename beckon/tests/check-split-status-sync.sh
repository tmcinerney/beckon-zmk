#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/../.." && pwd)

types="$root/app/include/zmk/split/transport/types.h"
service="$root/app/src/split/bluetooth/service.c"
central="$root/app/src/split/bluetooth/central.c"
transport="$root/beckon/config/transport.conf"
config="$root/beckon/config/default.nix"

# The forwarded payload is deliberately a compact, fixed-size snapshot. Keep
# it independent of the host's 32-byte USB report and never add an input path.
grep -Fq '#define ZMK_SPLIT_TRANSPORT_BECKON_STATUS_SLOT_COUNT 10' "$types"
grep -Fq 'ZMK_SPLIT_TRANSPORT_CENTRAL_CMD_TYPE_SET_BECKON_STATUS' "$types"
grep -Fq 'BT_GATT_PERM_WRITE_ENCRYPT' "$service"
grep -Fq 'zmk_split_central_resync_beckon_status' "$central"

# Raw HID remains a central-only USB transport. The right build has no
# transport.conf and the configuration never enables the raw BLE service.
grep -Fq 'kconfig = ./beckon.conf;' "$config"
if grep -Eq '^CONFIG_RAW_HID_BLE=y$' "$transport" "$root/beckon/config/beckon.conf"; then
  echo "Beckon split sync must not enable Raw HID over Bluetooth" >&2
  exit 1
fi

echo "Beckon split status sync invariants passed"
