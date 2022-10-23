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
        , fetchFromGitHub
        , cmake
        , fmt_latest
        , openssl
        }:
        let
          # Kept in sync with submodules, make sure to update this accordingly.
          submodules = {
            erdb = fetchFromGitHub {
              owner = "EldenRingDatabase";
              repo = "erdb";
              rev = "87e931605b137f320fae894bddb139aaa54615cb";
              sha256 = "sha256-1eauIs6RzzFoElF3IAywGPIgsD+4mlKxENx8TtWNMFU=";
            };
          };

          copySubmodules = lib.concatMapStringsSep "\n"
            (submodule: ''
              mkdir -p external/${submodule}
              cp -r ${submodules.${submodule}}/* external/${submodule}
            '')
            (builtins.attrNames submodules);

          buildDate =
            let
              year = lib.substring 0 4 self.lastModifiedDate;
              month = lib.substring 4 2 self.lastModifiedDate;
              day = lib.substring 6 2 self.lastModifiedDate;
            in
            "${year}-${month}-${day}";
        in
        clang13Stdenv.mkDerivation rec {
          pname = "erutils";
          version = "0.pre+date=${buildDate}";

          src = lib.cleanSourceWith {
            src = lib.cleanSource self;
            filter = name: type:
              !(baseNameOf name == "build" && type == "directory") && !(baseNameOf name == "generateditems.h" && type == "file");
          };

          nativeBuildInputs = [
            cmake
          ];

          buildInputs = [
            fmt_latest
            openssl
          ];

          cmakeFlags = [
            "-DVERSION=${buildDate}"
          ];

          preConfigure = copySubmodules;

          doInstallCheck = true;
          installCheckPhase = ''
            ./erutils --version | grep -q 'v${buildDate}' || exit 1
            echo "found version 'v${buildDate}'";
          '';

          meta = with lib; {
            inherit description;
            homepage = "https://github.com/IvarWithoutBones/erutils";
            license = licenses.asl20;
            platforms = platforms.unix;
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
