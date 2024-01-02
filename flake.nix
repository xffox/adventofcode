{
  description = "adventofcode";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    test-riddle = {
      url = "github:xffox/test-riddle";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { self, nixpkgs, flake-utils, test-riddle, ... }:
  flake-utils.lib.eachDefaultSystem (system :
  let
    pkgs = nixpkgs.legacyPackages.${system};
    testPkgs = {test-riddle = test-riddle.packages.${system}.default;};
    makeShell = ({stdenv} : pkgs.callPackage ./shell.nix {inherit testPkgs; inherit stdenv; });
    adventofcodePkgs = pkgs.callPackage ./build.nix {stdenv=pkgs.gcc13Stdenv; inherit testPkgs;};
  in {
    packages = {
      adventofcode2023 = adventofcodePkgs.adventofcode2023;
    };
    devShells = {
      default = makeShell {stdenv = pkgs.gcc13Stdenv;};
      staticBuild = makeShell {stdenv = pkgs.pkgsStatic.gcc13Stdenv;};
      clangBuild = makeShell {stdenv = pkgs.clang16Stdenv;};
    };
  });
}
