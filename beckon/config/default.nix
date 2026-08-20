{ firmware ? import ../.. {} }:

let
  left = firmware.glove80_left.override {
    keymap = ./beckon.keymap;
    kconfig = ./beckon.conf;
  };
  right = firmware.glove80_right.override {
    keymap = ./beckon.keymap;
    kconfig = ./beckon.conf;
  };
in
firmware.combine_uf2 left right "glove80"
