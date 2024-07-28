{ pkgs ? import <nixpkgs> {}
}:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    arduino-cli
    python3
    python311Packages.pyserial
    arduino-language-server
  ];

  shellHook = ''
    LOCALE_ARCHIVE="$(nix-build --no-out-link '<nixpkgs>' -A glibcLocales)/lib/locale/locale-archive"

    export LANG=en_GB.UTF-8
  '';
}
