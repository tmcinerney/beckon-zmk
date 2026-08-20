# Beckon firmware agent guidance

## Branches and worktrees

- `main` is a clean mirror of `moergo-sc/zmk` upstream. Do not add Beckon work
  there and do not use it as the implementation checkout.
- `beckon/main` is the default branch and contains Beckon firmware, CI, release,
  and agent guidance.
- The root checkout at `~/Code/Private/beckon-zmk` stays on `main`.
- Make Beckon changes in the `beckon/main` worktree at
  `~/Code/.worktrees/beckon-zmk/beckon-main`, or in a worktree branched from it.
- `origin/HEAD` points to `origin/beckon/main`; fresh clones start there.

## Updating upstream

- The scheduled workflow proposes fast-forward updates to `main`.
- After a reviewed upstream update lands, rebase `beckon/main` onto `main`.
- Run the Beckon firmware workflow and physically test both halves before a
  release. Never auto-flash a keyboard.

## Firmware safety

- `beckon/config/beckon.keymap` is the source of truth for the current layout.
- Do not commit generated `.uf2` files. Keep a known-good recovery UF2 outside
  the repository.
- Do not use `nrf52840-nosd` snippets or raise the RGB brightness safety limit.
