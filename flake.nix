rec {
  description = "A Elden Ring save file editor for the command line";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { self
    , nixpkgs
    , flake-utils
    }:
    let
      erutils-drv =
        { lib
        , clang13Stdenv
        , cmake
        , fmt_latest
        , openssl_1_1
        }:
        clang13Stdenv.mkDerivation rec {
          pname = "erutils";
          version = "0.0.1";

          src = lib.cleanSourceWith {
            src = lib.cleanSource ./.;
            filter = name: type: !(baseNameOf name == "build" && type == "directory");
          };

          nativeBuildInputs = [
            cmake
          ];

          buildInputs = [
            fmt_latest
            openssl_1_1
          ];

          doInstallCheck = true;

          installCheckPhase = ''
            ./erutils --version | grep -q 'v${version}' || exit 1
            echo "found version 'v${version}'";
          '';

          meta = with lib; {
            inherit description;
            homepage = "https://github.com/IvarWithoutBones/erutils";
            license = licenses.asl20;
            platforms = platforms.linux;
          };
        };
    in
    {
      overlays =
        let
          erutils = final: prev: {
            erutils = final.callPackage erutils-drv { };
          };
        in
        {
          inherit erutils;
          default = erutils;
        };
    } // flake-utils.lib.eachDefaultSystem (system:
    let
      inherit (nixpkgs.legacyPackages.${system}) callPackage;
      erutils = callPackage erutils-drv { };
    in
    {
      packages = {
        inherit erutils;
        default = erutils;
      };
    });
}
