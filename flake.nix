rec {
  description = "A utility to patch different Steam IDs into Elden Ring savefiles";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      forAllSystems = nixpkgs.lib.genAttrs [ "x86_64-linux" ];

      # Nixpkgs instantiated for each supported system
      nixpkgsFor = forAllSystems (system:
        import nixpkgs {
          inherit system;
          overlays = [ self.overlays.default ];
        });
    in
    {
      overlays.default = final: prev: rec {
        erutils = with final; clang13Stdenv.mkDerivation rec {
          pname = "erutils";
          version = "0.0.1";

          src = self;
          doInstallCheck = true;

          nativeBuildInputs = [
            cmake
          ];

          buildInputs = [
            fmt_latest
            openssl_1_1
          ];

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
      };

      packages = forAllSystems (system: {
        inherit (nixpkgsFor.${system}) erutils;
        default = (nixpkgsFor.${system}).erutils;
      });
    };
}
