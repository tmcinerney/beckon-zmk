{ pkgs ? import ../nix/pinned-nixpkgs.nix {} }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    bash clang coreutils jq actionlint shellcheck pre-commit
    (python3.withPackages (ps: [ ps.pyyaml ps.west ]))
  ];
}
