#!/usr/bin/env bash
set -euo pipefail

keymap="${1:-beckon/config/beckon.keymap}"

require() {
  if ! grep -Fq -- "$1" "$keymap"; then
    echo "missing expected Beckon layout fragment: $1" >&2
    exit 1
  fi
}

require '#define LAYER_Beckon 4'
require '#define LAYER_BeckonMagic 5'
require '&kp F16         &kp F17  &kp F18   &kp F19    &kp F20'
require '&kp LS(F16)  &kp LS(F17)  &kp LS(F18)  &kp LS(F19)  &kp LS(F20)'
require '&to LAYER_Beckon'
require '&to LAYER_Base'
require '&magic LAYER_BeckonMagic 0'
require 'layer_BeckonMagic {'

echo "Beckon keymap invariants passed: $keymap"
