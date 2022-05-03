{
  description = "A work-in-progresss TUI task manager for Linux written in C++";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs }: let
    version = builtins.substring 0 8 self.lastModifiedDate;

    # All supported systems
    forAllSystems = nixpkgs.lib.genAttrs [ "x86_64-linux" "aarch64-linux" ];

    # Nixpkgs instantiated for each supported system
    nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; overlays = [ self.overlays.default ]; });
  in {
    overlays.default = final: prev: {
      tuitop = with final; clangStdenv.mkDerivation rec {
        pname = "tuitop";
        inherit version;

        src = ./.;

        nativeBuildInputs = [
          cmake
          pkg-config
        ];

        buildInputs = [
          ftxui
          fmt
          procps
        ];

        meta = with lib; {
          homepage = "https://github.com/IvarWithoutBones/tuitop";
          description = "A work-in-progresss TUI task manager for Linux written in C++";
          license = licenses.gpl3Only;
          platforms = platforms.linux;
        };
      };
    };

    packages = forAllSystems (system: {
      inherit (nixpkgsFor.${system}) tuitop;
      default = (nixpkgsFor.${system}).tuitop;
    });
  };
}
