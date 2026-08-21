# Beckon status transport v1

This is a one-way, host-to-keyboard status protocol. It has no command for key
input, behavior invocation, approval, or agent control.

The `zmk-raw-hid` endpoint uses vendor page `0xFF60`, usage `0x61`, and a
fixed 32-byte output report. It is enabled only on Glove80's left, split-central
half. The right half remains a ZMK split peripheral. The central forwards a
compact copied snapshot to it through the existing encrypted ZMK split service;
the right half never exposes Raw HID to the host or over Bluetooth.

| Byte(s) | Meaning                               |
| ------- | ------------------------------------- |
| 0       | Protocol version: `1`                 |
| 1       | Message type: `1` (complete snapshot) |
| 2       | Host sequence number, modulo 256      |
| 3       | Reserved, must be `0`                 |
| 4–13    | Agent state for F1 through F10        |
| 14–31   | Reserved, must be `0`                 |

Agent state values are `0` unbound, `1` idle, `2` working, `3` blocked, `4`
done, and `5` unknown. The firmware rejects every malformed or unsupported
frame. A valid snapshot is copied into a ZMK event for a later RGB renderer;
this increment intentionally does not modify LEDs.

## Physical gate

Compilation establishes only the source-level integration. Before enabling this
in a release candidate, flash both halves with the guided installer and verify:

1. The keyboard presents a second USB HID interface while wired.
2. A host writes a valid 32-byte snapshot to that interface.
3. Invalid frames are ignored and ordinary key input remains unaffected.
4. A `working` state in slot F6 turns the right F6 proof LED teal, while F1
   remains unchanged.
