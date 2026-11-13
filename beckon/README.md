# Beckon Glove80 firmware

This branch contains the firmware part of Beckon. It is a public fork of
[`moergo-sc/zmk`](https://github.com/moergo-sc/zmk), with MoErgo's unmerged
per-key/layer RGB work ported onto `beckon/main` and a reproducible copy of the
current Glove80 Layout Editor keymap.

`beckon/config/beckon.keymap` is the source of truth for the keyboard mapping.
The `.uf2` currently in Downloads is a recovery artifact, not source and must
not be committed.

## Current Beckon layer

The persistent `Beckon` layer is layer 4. The key beside Magic toggles Base and
Beckon. Holding Magic from Beckon activates a private priority-5 overlay so
Magic controls, including RGB, remain available. On the Beckon layer the F-row
emits these hotkeys:

| Physical key | Host key            |
| ------------ | ------------------- |
| F1–F5        | F16–F20             |
| F6–F10       | Shift+F16–Shift+F20 |

The mapping avoids macOS's built-in F1–F15 display/media handling.

## Build and test

CI uses the upstream ZMK 3.5 build container and one shared build entry point:

```sh
./beckon/scripts/build-west.sh
```

Run it inside the documented Linux/Docker builder with West and the Zephyr
SDK available. On macOS, use GitHub Actions or the optional Nix expression in
`beckon/config/default.nix`; the pinned Nix tool environment can run the
host-side checks without a device:

```sh
nix-shell beckon/shell.nix
python3 beckon/scripts/check-dependency-pins.py
python3 -m unittest discover -s beckon/tests -p 'test_release.py' -v
./beckon/tests/test-checks.sh
```

`app/west.yml` and `nix/manifest.json` must pin the same immutable direct
revisions. The shared builder checks both the declarations and fetched Git
HEADs. Both build routes apply the raw-HID USB-only patch and MoErgo's Zephyr
input deadlock backport. Updating a dependency requires updating both pins,
its Nix source hash, and any patch that no longer applies.

A successful build emits `beckon/glove80.uf2` (both halves), `build.log`,
`source.json` (the actual checkout SHA), and `west-frozen.yml` (resolved
transitive dependencies). The shell uses `pipefail`: a failed compiler command
cannot be hidden by `tee`.

The checks deliberately cover different failure modes:

- GitHub Actions uses the upstream ZMK build container, compiles both halves,
  and uploads the combined UF2.
- `check-keymap.sh` prevents a silent loss of the Beckon layer, mappings, or
  Base/Beckon toggle.
- `check-memory.sh` rejects builds above conservative 60% flash or 50% RAM
  guardrails.
- `test-status-protocol.sh` exercises the strict, status-only Raw HID frame
  parser outside a device build.

## Status transport and ten-key LED renderer

The left Glove80 half is the USB-connected split central. It exposes a second,
vendor-defined Raw HID interface used only for a fixed-size status snapshot.
The right half exposes no host transport. The central forwards a compact status
snapshot through ZMK's existing encrypted split service, which re-sends the
last snapshot after the right half reconnects.

The firmware receives and validates the frame, then publishes a copied ZMK
event. The host protocol contains only fixed-size status enums; it cannot alter
key behavior.

The display renderer consumes every snapshot slot: F1–F5 on the USB-connected
left half and F6–F10 on the split peripheral. Each overrides its LED with a
host-selected status treatment. The original default colors are:

| Agent state | F1 / F6 color     |
| ----------- | ----------------- |
| idle        | blue (`#3BA0FF`)  |
| working     | teal (`#00C48C`)  |
| blocked     | red (`#FF2B2B`)   |
| done        | white (`#FFFFFF`) |
| unknown     | amber (`#FFB000`) |
| unbound     | no override       |

Protocol v2 carries five RGB/brightness/motion treatments and ten packed status slots.
The firmware renders steady, breathe, pulse, or flicker locally; animation does not
require a stream of host reports. The Glove80 board DTS
owns physical LED wiring. Its `pixel-lookup` maps a WS2812 strip index to a
matrix position. `check-status-led-mapping.sh` verifies all ten F-row
relationships, so a keymap or board mapping change cannot silently move an
indicator. Status colours are scaled to a maximum 80% brightness before the
override is written, matching MoErgo's Glove80 RGB safety limit; this is
independent of host data and prevents a status such as white `done` from
bypassing the board limit.

The override composes over normal per-key layer RGB, respects `RGB_OFF`, and
yields to Magic's temporary keyboard-status display. Keep RGB enabled when
performing the physical proof.

After flashing a candidate, enable RGB, send a valid snapshot with selected
slots set to `working`, and confirm the matching keys become teal. Repeat with
`blocked` (red), then send `unbound` and confirm each key returns to normal
layer RGB. Hold Magic to show normal keyboard status, release it, and confirm
the overrides return. Switching out of Beckon layer 4 clears all overrides.

The protocol and broader physical acceptance criteria are in
[transport-protocol.md](transport-protocol.md).

## Deferred follow-up: wireless battery impact

Characterize Beckon's battery impact before enabling host status over Bluetooth.
Compare stock and Beckon firmware under the same wireless connection, RGB
brightness/effect, typing workload, and sleep settings. Record battery level
or, preferably, inline power-meter current over a long enough interval to
separate the status transport cost from the much larger RGB baseline.

## Recovery and release safety

An invalid ordinary ZMK image is recoverable using the Glove80 bootloader
power-up combo. Keep a known-good Layout Editor UF2 outside this repository.
Do not use `nrf52840-nosd` snippets or change the RGB brightness safety limit.

## Candidate, flash, and promote

The release path deliberately separates a build from a physical test:

Every checked push to `beckon/main` automatically publishes a prerelease
`v0.2.0-rc.<run-id>`. Set the repository variable `BECKON_RELEASE_TAG` when the
next target version changes. Only the custom branch publishes
automatically; PR and feature-branch builds produce artifacts for review.

The build must pass workflow/shell lint, repository pre-commit, release regression
tests, both firmware builds, protocol/renderer/keymap tests, USB-only checks, and
memory limits. Every candidate also runs the upstream tests, BLE simulator, Nix builds, and
docs checks as reusable workflows at the same source SHA. Native job dependencies
block publication on any failure, even for a workflow-only change. Automatic and manual candidates use the same build workflow.
Each release includes the combined UF2, SHA-256, exact source commit, build log,
frozen dependencies, and physical smoke checklist. A commit comment mentions
`BECKON_NOTIFY_USER` (repository owner by default) with the download link, and
the Actions summary links the candidate. GitHub controls inbox/email delivery
through your notification settings. A retry of the same source/tag verifies
existing metadata before succeeding and does not repeat the mention.

A candidate is ready to **install and test**, not already physically accepted.
Final promotion still requires your explicit physical-test confirmation.

For a manually selected tag (including a candidate from an explicit feature ref):

1. Choose the intended final tag, `vX.Y.Z`. The candidate metadata records
   that tag and the exact built source commit, so no version-only source commit is
   needed.
2. Run **Prepare Beckon firmware candidate** with `vX.Y.Z` and
   `vX.Y.Z-rc.1`. It builds, tests, and publishes a prerelease containing the
   combined UF2, SHA-256 checksum, source commit, and smoke-test checklist.
3. On macOS, run:

   ```sh
   ./beckon/scripts/install-candidate-macos.sh --candidate vX.Y.Z-rc.1
   ```

   The installer downloads the candidate, validates its SHA-256, guides the
   right half then left half into the reliable power-up bootloader mode, waits
   for `GLV80RHBOOT` and `GLV80LHBOOT`, and copies a short `BECKON.UF2`
   filename. An automatic bootloader-volume eject is treated as success even
   if macOS reports a late copy error. Use `--dry-run` to download and verify
   without touching the keyboard.

4. Complete the printed physical smoke test. The helper cannot prove that the
   keyboard has booted the new image; this is intentionally a human check.
5. Run **Promote Beckon firmware candidate** with the tested candidate tag,
   final tag, and physical-test confirmation. It verifies the candidate asset
   and metadata, then publishes the exact tested UF2 without rebuilding.

The installer never factory-resets or enters bootloader mode on your behalf.
MoErgo recommends its power-up bootloader mode because it works even with a
bad ZMK image or disconnected halves. Keep a known-good Layout Editor UF2
outside the repository. Do not use `nrf52840-nosd` snippets or change the RGB
brightness safety limit.

## Upstream updates

`main` is an upstream mirror. `beckon/main` contains Beckon-specific commits.
The weekly **Check MoErgo upstream** job opens a PR only when `upstream/main`
has advanced. After reviewing and merging it:

```sh
git switch beckon/main
git merge main
git push
```

Run the firmware CI and a physical smoke test after every upstream merge. A conflict is
useful signal that the RGB port needs an intentional update, not something an
automation should guess through.

## One-time GitHub setup

In **Settings → Actions → General**, permit workflows to create pull requests
and write repository contents. The scheduled upstream check needs this to open
its proposal; the manually triggered release workflow needs it to create a tag
and release. Both workflows otherwise fail closed.

After the first successful Beckon firmware run, protect `beckon/main` and require the firmware build
and checks before merging. Do not let automation push directly to
either `main` or `beckon/main`.
