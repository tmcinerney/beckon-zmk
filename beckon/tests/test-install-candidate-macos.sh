#!/usr/bin/env bash
set -euo pipefail

help=$(./beckon/scripts/install-candidate-macos.sh --help)
[[ "$help" == *"GLV80RHBOOT"* ]]
[[ "$help" == *"GLV80LHBOOT"* ]]

test_root=$(mktemp -d)
trap 'rm -rf "$test_root"' EXIT
mock_bin="$test_root/bin"
volumes_root="$test_root/volumes"
flash_work_dir="$test_root/flash-work"
mkdir -p "$mock_bin" "$flash_work_dir" "$volumes_root/GLV80RHBOOT" "$volumes_root/GLV80LHBOOT"
touch "$volumes_root/GLV80RHBOOT/INFO_UF2.TXT" "$volumes_root/GLV80LHBOOT/INFO_UF2.TXT"

write_mock() {
  local name="$1"
  shift
  printf '%s\n' '#!/usr/bin/env bash' "$@" > "$mock_bin/$name"
  chmod +x "$mock_bin/$name"
}

write_mock uname 'printf "Darwin\\n"'
write_mock mktemp 'printf "%s\\n" "$BECKON_TEST_FLASH_DIR"'
write_mock gh '
destination=""
while (($#)); do
  case "$1" in
    --dir) destination="$2"; shift 2 ;;
    *) shift ;;
  esac
done
mkdir -p "$destination"
printf firmware > "$destination/beckon.uf2"
printf "{\\n  \\"release_tag\\": \\"v0.1.0\\"\\n}\\n" > "$destination/candidate.json"
touch "$destination/SHA256SUMS" "$destination/SMOKE-TEST.md"
'
write_mock shasum 'printf "beckon.uf2: OK\\n"'
write_mock cp '
/bin/cp "$@"
destination="${!#}"
rm -rf "$(dirname "$destination")"
'
write_mock sync ':'
write_mock sleep ':'

output=$(printf '\n\n' | env PATH="$mock_bin:$PATH" BECKON_TEST_FLASH_DIR="$flash_work_dir" BECKON_VOLUMES_ROOT="$volumes_root" \
  ./beckon/scripts/install-candidate-macos.sh --candidate test)
[[ "$output" == *"right bootloader ejected: flash accepted."* ]]
[[ "$output" == *"left bootloader ejected: flash accepted."* ]]
