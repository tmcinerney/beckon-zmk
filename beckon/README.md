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
Beckon. On the Beckon layer the F-row emits these hotkeys:

| Physical key | Host key            |
| ------------ | ------------------- |
| F1–F5        | F16–F20             |
| F6–F10       | Shift+F16–Shift+F20 |

The mapping avoids macOS's built-in F1–F15 display/media handling.

## Build and test

CI uses MoErgo/ZMK's normal `west` build image, rather than Nix. The equivalent
commands are:

```sh
west init -l app
west update --fetch-opt=--filter=tree:0
west zephyr-export
west build -s app -d build/beckon-lh -b glove80_lh -- \
  -DKEYMAP_FILE="$PWD/beckon/config/beckon.keymap" \
  -DEXTRA_CONF_FILE="$PWD/beckon/config/beckon.conf"
west build -s app -d build/beckon-rh -b glove80_rh -- \
  -DKEYMAP_FILE="$PWD/beckon/config/beckon.keymap" \
  -DEXTRA_CONF_FILE="$PWD/beckon/config/beckon.conf"
cat build/beckon-lh/zephyr/zmk.uf2 build/beckon-rh/zephyr/zmk.uf2 > beckon/glove80.uf2
./beckon/tests/check-keymap.sh
```

The output is one combined `beckon/glove80.uf2`. Flash that same file to
each half's bootloader drive. Never flash an unreviewed CI artifact directly.

For local Mac builds, use the documented Linux/Docker builder from MoErgo, with
the checkout under `~/Code` rather than `/private/tmp`, or use GitHub Actions.
The optional Nix expression in `beckon/config/default.nix` remains useful for a
reproducible local build, but CI deliberately does not depend on it.

The checks deliberately cover different failure modes:

- GitHub Actions uses the upstream ZMK build container, compiles both halves,
  and uploads the combined UF2.
- `check-keymap.sh` prevents a silent loss of the Beckon layer, mappings, or
  Base/Beckon toggle.
- `check-memory.sh` rejects builds above conservative 60% flash or 50% RAM
  guardrails.
- Future host-controlled HID code must add Zephyr `ztest` coverage for packet
  validation, state rendering, invalid packets, and split-side propagation.

## Recovery and release safety

An invalid ordinary ZMK image is recoverable using the Glove80 bootloader
power-up combo. Keep a known-good Layout Editor UF2 outside this repository.
Do not use `nrf52840-nosd` snippets or change the RGB brightness safety limit.

## Candidate, flash, and promote

The release path deliberately separates a build from a physical test:

1. Choose the intended final tag, `vX.Y.Z`. The candidate metadata records
   that tag and the exact source commit, so no version-only source commit is
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
git rebase main
git push --force-with-lease
```

Run the firmware CI and a physical smoke test after every rebase. A conflict is
useful signal that the RGB port needs an intentional update, not something an
automation should guess through.

## One-time GitHub setup

In **Settings → Actions → General**, permit workflows to create pull requests
and write repository contents. The scheduled upstream check needs this to open
its proposal; the manually triggered release workflow needs it to create a tag
and release. Both workflows otherwise fail closed.

After the first successful Beckon firmware run, protect `beckon/main` and make
that workflow a required status check. Do not let automation push directly to
either `main` or `beckon/main`.
