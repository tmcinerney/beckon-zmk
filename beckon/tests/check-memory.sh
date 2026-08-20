#!/usr/bin/env bash
set -euo pipefail

# The build log reports one FLASH and RAM line per Glove80 half. These are
# intentionally conservative guardrails, not a claim about the true limit.
log_file="${1:?usage: check-memory.sh BUILD_LOG}"
flash_limit=60
ram_limit=50

check_usage() {
  local label="$1" limit="$2"
  local values
  values=$(awk -v label="$label" '$0 ~ label ":" { value=$NF; sub(/%$/, "", value); print value }' "$log_file")

  if [[ -z "$values" ]]; then
    echo "no $label usage found in $log_file" >&2
    exit 1
  fi

  while read -r value; do
    if awk -v value="$value" -v limit="$limit" 'BEGIN { exit !(value > limit) }'; then
      echo "$label usage ${value}% exceeds ${limit}% guardrail" >&2
      exit 1
    fi
  done <<< "$values"
}

check_usage FLASH "$flash_limit"
check_usage RAM "$ram_limit"
echo "Firmware memory guardrails passed"
