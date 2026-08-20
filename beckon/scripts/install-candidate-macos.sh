#!/usr/bin/env bash
set -euo pipefail

repo="tmcinerney/beckon-zmk"
candidate=""
dry_run=false
volumes_root="/Volumes"

usage() {
  cat <<'EOF'
Usage: install-candidate-macos.sh --candidate TAG [--repo OWNER/REPO] [--dry-run]

Downloads a Beckon prerelease, verifies SHA-256, then guides flashing the same
combined UF2 to the right and left Glove80 bootloader volumes. It never enters
bootloader mode or factory-resets either half; those remain physical actions.

Expected macOS volumes: GLV80RHBOOT then GLV80LHBOOT.
EOF
}

while (($#)); do
  case "$1" in
    --candidate) candidate="${2:?missing candidate tag}"; shift 2 ;;
    --repo) repo="${2:?missing repository}"; shift 2 ;;
    --dry-run) dry_run=true; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "unknown argument: $1" >&2; usage >&2; exit 2 ;;
  esac
done

if [[ -z "$candidate" ]]; then
  echo "--candidate is required" >&2
  usage >&2
  exit 2
fi

if [[ "$(uname)" != "Darwin" ]]; then
  echo "this installer is macOS-only" >&2
  exit 2
fi

for command in gh shasum cp; do
  command -v "$command" >/dev/null || {
    echo "required command not found: $command" >&2
    exit 2
  }
done

work_dir=$(mktemp -d -t beckon-flash)
trap 'rm -rf "$work_dir"' EXIT

echo "Downloading Beckon candidate $candidate from $repo..."
gh release download "$candidate" --repo "$repo" --pattern beckon.uf2 --pattern SHA256SUMS \
  --pattern candidate.json --pattern SMOKE-TEST.md --dir "$work_dir"

(
  cd "$work_dir"
  shasum -a 256 -c SHA256SUMS
)

echo "Verified candidate: $(sed -n 's/.*"release_tag": "\([^"]*\)".*/\1/p' "$work_dir/candidate.json")"

if $dry_run; then
  echo "Dry run complete. Candidate assets are valid; no keyboard was flashed."
  exit 0
fi

wait_for_bootloader() {
  local hand="$1" volume="$2" instruction="$3"
  local mount="$volumes_root/$volume"

  printf '\n%s\n' "Flash the $hand half"
  printf '%s\n' "$instruction"
  printf '%s' "When its red LED pulses slowly and $volume appears, press Return... "
  read -r _

  until [[ -d "$mount" && -f "$mount/INFO_UF2.TXT" ]]; do
    printf '%s\n' "Waiting for $mount (check cable, power, and bootloader mode)..."
    sleep 1
  done
}

flash_half() {
  local hand="$1" volume="$2" mount="$volumes_root/$volume"
  local copy_status=0

  echo "Copying verified firmware to $hand ($volume)..."
  cp "$work_dir/beckon.uf2" "$mount/BECKON.UF2" || copy_status=$?
  sync || true
  sleep 2

  if [[ ! -d "$mount" ]]; then
    echo "$hand bootloader ejected: flash accepted."
    return 0
  fi

  if ((copy_status)); then
    echo "$hand copy failed and the bootloader volume is still mounted." >&2
    return "$copy_status"
  fi

  echo "$hand bootloader did not eject after the copy; do not continue." >&2
  echo "Disconnect/retry with a direct cable, or inspect $mount before trying again." >&2
  return 1
}

wait_for_bootloader "right" "GLV80RHBOOT" \
  "Connect USB directly to the right half. Power it off, hold C6R6+C3R3 (I+PgDn on the default layout), then power it on."
flash_half "right" "GLV80RHBOOT"

wait_for_bootloader "left" "GLV80LHBOOT" \
  "Connect USB directly to the left half. Power it off, hold C6R6+C3R3 (Magic+E on the default layout), then power it on."
flash_half "left" "GLV80LHBOOT"

printf '\nFlashing is complete. Run the following physical checks before promotion:\n\n'
cat "$work_dir/SMOKE-TEST.md"
