rec {
  description = "A small program to convert Elden Ring save files";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs }: let
    version = builtins.substring 0 8 self.lastModifiedDate;

    forAllSystems = nixpkgs.lib.genAttrs [ "x86_64-linux" ];

    # Nixpkgs instantiated for each supported system
    nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; overlays = [ self.overlays.default ]; });
  in {
    overlays.default = final: prev: {
      er-savepatcher = with final; clangStdenv.mkDerivation rec {
        pname = "er-savepatcher";
        inherit version;

        src = self;

        nativeBuildInputs = [
          cmake
        ];

        buildInputs = [
          fmt
        ];

        meta = with lib; {
          inherit description;
          homepage = "https://github.com/IvarWithoutBones/er-savepatcher";
          license = licenses.mit;
          platforms = platforms.linux;
        };
      };
    };

    packages = forAllSystems (system: {
      inherit (nixpkgsFor.${system}) er-savepatcher;
      default = (nixpkgsFor.${system}).er-savepatcher;
    });
  };
}
