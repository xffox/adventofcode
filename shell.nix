{ pkgs, cmake, clang-tools_16, valgrind, python3,
  graphviz, wxmaxima,
  stdenv, testPkgs }:
pkgs.mkShell.override {stdenv=stdenv;} {
  packages = [
    cmake
    clang-tools_16
    valgrind
    (python3.withPackages (p : with p; [
      ipython
      sympy
      numpy
      python-lsp-server
      pyflakes
      flake8
      pylint
    ]))
    testPkgs.test-riddle
    graphviz
    wxmaxima
  ];
}
