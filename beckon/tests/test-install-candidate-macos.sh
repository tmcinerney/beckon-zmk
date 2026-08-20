#!/usr/bin/env bash
set -euo pipefail

help=$(./beckon/scripts/install-candidate-macos.sh --help)
[[ "$help" == *"GLV80RHBOOT"* ]]
[[ "$help" == *"GLV80LHBOOT"* ]]
