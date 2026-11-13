#!/usr/bin/env bash
set -euo pipefail

directory="${1:?usage: publish-candidate.sh DIRECTORY}"
repo="${GITHUB_REPOSITORY:?GITHUB_REPOSITORY is required}"
tag=$(jq -er '.candidate_tag' "$directory/candidate.json")
commit=$(jq -er '.commit | select(test("^[0-9a-f]{40}$"))' "$directory/candidate.json")
release_tag=$(jq -er '.release_tag' "$directory/candidate.json")
[[ "$release_tag" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]
[[ "$tag" =~ ^v[0-9]+\.[0-9]+\.[0-9]+-rc\.[0-9]+$ && "$tag" == "$release_tag"-rc.* ]]
(cd "$directory" && sha256sum --check SHA256SUMS)
url="https://github.com/$repo/releases/tag/$tag"

# AIDEV-NOTE: source identity comes from the built artifact, never a moving branch.
# An existing release must match before a retry can notify or report success.
if gh release view "$tag" --repo "$repo" --json tagName >/dev/null 2>&1; then
  actual=$(gh api "repos/$repo/commits/$tag" --jq .sha)
  [[ "$actual" == "$commit" ]]
  existing=$(mktemp -d)
  trap 'rm -rf "$existing"' EXIT
  gh release download "$tag" --repo "$repo" --pattern candidate.json --pattern SHA256SUMS --dir "$existing"
  cmp "$directory/candidate.json" "$existing/candidate.json"
  cmp "$directory/SHA256SUMS" "$existing/SHA256SUMS"
else
  gh release create "$tag" --repo "$repo" --target "$commit" --prerelease \
    "$directory/beckon.uf2" "$directory/SHA256SUMS" "$directory/candidate.json" \
    "$directory/SMOKE-TEST.md" "$directory/build.log" "$directory/west-frozen.yml" \
    --title "Beckon $tag" \
    --notes "Automated build and checks passed. Ready to install and physically test on BOTH halves. USB status transport; BLE status is not enabled. Promote to $release_tag only after completing SMOKE-TEST.md."
fi

if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
  printf '### Glove80 candidate ready\n\n[Download %s](%s). Automated checks passed; physical testing is still required.\n' "$tag" "$url" >> "$GITHUB_STEP_SUMMARY"
fi
# A GitHub mention provides a notification without another service or secret.
notify_user="${BECKON_NOTIFY_USER:-${repo%%/*}}"
marker="<!-- beckon-candidate:$tag -->"
comments=$(gh api --paginate "repos/$repo/commits/$commit/comments" --jq '.[].body')
if ! grep -Fq "$marker" <<< "$comments"; then
  body=$(printf '%s\n@%s Glove80 firmware candidate [%s](%s) passed automated build/tests and is ready to install and test on both halves. Final release promotion still requires the physical smoke test.' "$marker" "$notify_user" "$tag" "$url")
  gh api "repos/$repo/commits/$commit/comments" --method POST -f body="$body" >/dev/null
fi
printf 'Candidate ready: %s\n' "$url"
