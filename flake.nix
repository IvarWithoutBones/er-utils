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
        , openssl
        }:
        clang13Stdenv.mkDerivation rec {
          pname = "erutils";
          version =
            let
              year = lib.substring 0 4 self.lastModifiedDate;
              month = lib.substring 4 2 self.lastModifiedDate;
              day = lib.substring 6 2 self.lastModifiedDate;
            in
            "0.pre+date=${year}-${month}-${day}";

          src = lib.cleanSourceWith {
            src = lib.cleanSource ./.;
            filter = name: type: !(baseNameOf name == "build" && type == "directory");
          };

          nativeBuildInputs = [
            cmake
          ];

          buildInputs = [
            fmt_latest
            openssl
          ];

          cmakeFlags = [
            "-DVERSION=${version}"
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
