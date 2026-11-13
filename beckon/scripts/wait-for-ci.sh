#!/usr/bin/env bash
set -euo pipefail

commit="${1:?usage: wait-for-ci.sh COMMIT}"
repo="${GITHUB_REPOSITORY:?GITHUB_REPOSITORY is required}"
for ((attempt=0; attempt<${BECKON_CI_ATTEMPTS:-80}; attempt++)); do
  runs=$(gh run list --repo "$repo" --commit "$commit" --limit 100 \
    --json databaseId,workflowName,status,conclusion)
  checks=$(jq '[.[] | select(.workflowName == "Tests" or .workflowName == "BLE Tests" or .workflowName == "Build Firmware" or .workflowName == "Docs Checks" or .workflowName == "pre-commit" or .workflowName == "Beckon workflow lint")] | group_by(.workflowName) | map(max_by(.databaseId))' <<< "$runs")
  if jq -e 'any(.[]; .status == "completed" and .conclusion != "success" and .conclusion != "skipped")' <<< "$checks" >/dev/null; then
    echo "Source CI failed; withholding the install candidate:" >&2
    jq '.[] | {workflowName, status, conclusion}' <<< "$checks" >&2
    exit 1
  fi
  if jq -e 'all(.[]; .status == "completed")' <<< "$checks" >/dev/null; then
    echo "All triggered source CI checks passed"
    exit 0
  fi
  echo "Waiting for source CI before publishing $commit"
  sleep "${BECKON_CI_INTERVAL_SECONDS:-15}"
done
echo "Timed out waiting for source CI; no candidate published" >&2
exit 1
