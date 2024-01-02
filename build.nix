{ stdenv, cmake, testPkgs }:
{
  adventofcode2023 = stdenv.mkDerivation {
    name = "adventofcode2023";
    src = ./2023;
    nativeBuildInputs = [
      cmake
    ];
    nativeCheckInputs = [
      testPkgs.test-riddle
    ];
    doCheck = true;
  };
}
