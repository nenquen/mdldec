{
  description = "mdldec - GoldSrc model decompiler (Linux/NixOS packaging)";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = {
    self,
    nixpkgs,
  }: let
    systems = [
      "aarch64-linux"
      "i686-linux"
      "x86_64-linux"
    ];
    forAllSystems = nixpkgs.lib.genAttrs systems;

    # Plain stdenv build rather than CMake: CMakeLists.txt is deliberately
    # MSVC-only (it hard-fails on any other compiler), and MSVC remains the
    # reference build via build.bat. This derivation is the POSIX path.
    mdldecFor = pkgs:
      pkgs.stdenv.mkDerivation rec {
        pname = "mdldec";
        version = "1.0.0";

        src = ./.;

        strictDeps = true;

        nativeBuildInputs = [pkgs.gnumake];

        # Only common.h-style headers come from src/, and the project links
        # winmm.lib on MSVC. Nothing here is used on POSIX: the link is dead
        # weight inherited from DecompMDL, so it is not carried over.
        buildPhase = ''
          runHook preBuild

          cc -std=c11 -O2 -Wall \
            -D_CRT_SECURE_NO_WARNINGS \
            -Isrc \
            src/*.c -o mdldec -lm

          runHook postBuild
        '';

        installPhase = ''
          runHook preInstall

          install -Dm755 mdldec $out/bin/mdldec

          runHook postInstall
        '';

        # exercise the built binary, not just the compiler
        doCheck = true;
        checkPhase = ''
          runHook preCheck

          # Invoked through bash explicitly: the build sandbox has no
          # /usr/bin/env, so the shebang is not resolvable there.
          # stdenv runs checkPhase before installPhase, so this tests the
          # binary still sitting in the build tree.
          bash test/smoke.sh -Exe "$PWD/mdldec"

          runHook postCheck
        '';

        meta = {
          description = "GoldSrc model decompiler (DecompMDL derivative)";
          # The Half-Life 1 SDK license is not OSI-approved, so this cannot
          # go into nixpkgs as-is; it is packaged for personal use only.
          license = nixpkgs.lib.licenses.unfree;
          platforms = systems;
          mainProgram = "mdldec";
        };
      };
  in {
    packages = forAllSystems (system: rec {
      mdldec = mdldecFor nixpkgs.legacyPackages.${system};
      default = self.packages.${system}.mdldec;
    });

    # The same derivation, built from a caller-supplied pkgs. The package is
    # unfree, and nixpkgs refuses to instantiate an unfree package unless
    # *that* pkgs set allows it. `inputs.<name>.follows` only pins the nixpkgs
    # source; it does not carry a `config.allowUnfree` across, so a consumer
    # that already allows unfree packages still has to inject its own pkgs.
    lib.mdldec = pkgs: mdldecFor pkgs;

    checks = forAllSystems (system: {
      mdldec = self.packages.${system}.mdldec;
    });

    devShells = forAllSystems (system: {
      default = nixpkgs.legacyPackages.${system}.mkShell {
        inputsFrom = [self.packages.${system}.mdldec];
        packages = [
          nixpkgs.legacyPackages.${system}.gdb
          nixpkgs.legacyPackages.${system}.valgrind
        ];
      };
    });

    formatter = forAllSystems (system: nixpkgs.legacyPackages.${system}.alejandra);
  };
}
