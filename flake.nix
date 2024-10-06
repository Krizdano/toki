{
  description = "Toki - command-line project manager for C and Assembly";

  inputs.nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";

  outputs = { self, nixpkgs, ... }:
    let
      supportedSystems = [
        "x86_64-linux"
        "i686-linux"
        "aarch64-linux"
      ];
      forAllSystems =
        f:
        nixpkgs.lib.genAttrs supportedSystems (
          system:
          f {
            pkgs = nixpkgs.legacyPackages.${system};
            inherit system;
          });
      in
      {
        packages = forAllSystems (
          { pkgs, system, ... }: {
            toki = pkgs.callPackage ./default.nix { };
            default = self.packages.${system}.toki;
          });
        devShells = forAllSystems (
          { pkgs, system, ... }: {
            default = pkgs.stdenvNoCC.mkDerivation {
              name = "toki-build-shell";
              buildInputs = with pkgs; [  gdb gcc gnumake nasm ];
            };
          });
      };
}
