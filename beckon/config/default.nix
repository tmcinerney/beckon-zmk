{ firmware ? import ../.. {} }:

let
  left = firmware.glove80_left.override {
    keymap = ./beckon.keymap;
    # Raw HID exists only on the split central (left) half. The right half
    # receives future display state over ZMK's split transport, never from USB.
    kconfig = builtins.concatStringsSep ";" [ ./beckon.conf ./transport.conf ];
    extraModules = [ ../module ];
  };
  right = firmware.glove80_right.override {
    keymap = ./beckon.keymap;
    kconfig = ./beckon.conf;
    extraModules = [ ../module ];
  };
in
firmware.combine_uf2 left right "glove80"
