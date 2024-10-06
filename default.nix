{ pkgs, fetchFromGitHub, ... }:

pkgs.stdenv.mkDerivation {
  name = "toki";
  verson = "0.2";
  src = ./.;

  buildPhase = ''
          make withflakesupport
          '' ;

  installPhase = ''
          # ls $out
          mkdir -p $out/bin
          cp build/bin/toki  $out/bin/toki
          '';
}
