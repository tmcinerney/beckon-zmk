# Beckon firmware candidate smoke test

Use `beckon/scripts/install-candidate-macos.sh --candidate <tag>` to download,
verify, and flash this candidate. It flashes the right half first, then left.

Before promotion, verify:

- Base layer typing works on both halves over the normal wired connection.
- The key beside Magic toggles Base and Beckon in both directions.
- Beckon F1–F5 send F16–F20; F6–F10 send Shift+F16–Shift+F20.
- The host hotkey listener receives all ten expected inputs.
- RGB can be enabled and both halves light correctly.
- The halves reconnect to each other and the normal host after flashing.

Do not factory-reset automatically. If the halves do not reconnect after this
firmware-version change, follow MoErgo's documented reset and re-pair procedure.
Once every check passes, run **Promote Beckon firmware candidate** with this
candidate tag, its target release tag, and physical-test confirmation.
