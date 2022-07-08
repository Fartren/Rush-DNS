let
  pkgs = import <nixpkgs> {};
  llvmPackages = pkgs.llvmPackages_14;
  # llvmPackages.stdenv pull GNU binutils but with want llvm variant
  # with mold instead of lld since it is faster.
  bintoolsWithMold = llvmPackages.bintools.override {
    bintools = llvmPackages.bintools.bintools.override {
      lld = pkgs.mold;
    };
  };
  clang = llvmPackages.clangUseLLVM.override {
    # llvmPackages.clangUseLLVM pull gcc but it seem uneeded
    gccForLibs = null;
    # TODO(Et7f3): Is mold 2.0 released ?
    bintools = if pkgs.mold.meta.broken
      then llvmPackages.clangUseLLVM.bintools
      else bintoolsWithMold;
  };
  clangStdenv = pkgs.overrideCC pkgs.stdenv clang;
  mkShell = pkgs.mkShell.override {
    stdenv = clangStdenv;
  };
in
  mkShell {
    name = "rush1";
    buildInputs = [
      pkgs.include-what-you-use
      pkgs.gitMinimal
      pkgs.json_c
      pkgs.pkg-config
    ];
    shellHooks = ''
      export MODE=DEV
      switchMode(){
        case "$MODE" in
          DEV)
            MODE=PROD;
          ;;
          PROD)
            MODE=DEV;
          ;;
        esac
      }
      export LD_LIBRARY_PATH=${pkgs.json_c}/lib:${llvmPackages.libunwind}/lib
    ''
    + pkgs.lib.optionalString (!pkgs.mold.meta.broken) ''
      export NIX_CFLAGS_COMPILE="$NIX_CFLAGS_COMPILE "-fuse-ld=mold
    '';
    # TODO(Et7f3): Debug this
    hardeningDisable = pkgs.lib.optional pkgs.targetPlatform.isDarwin "fortify";
  }
