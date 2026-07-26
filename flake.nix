{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-26.05-darwin";
    # nixpkgs-unstable has CMake 4.1.something, but our CMakeLists.txt
    # (optionally) uses features of CMake 4.3, so pull in the staging-next
    # channel, which already has a CMake 4.3.4 package.
    nixpkgs-staging.url = "github:NixOS/nixpkgs/staging-next";
    # this makes my devshells repo available, too, which contains some
    # utilities for creating devshells
    devshells.url = "github:ispeters/devshells";
  };

  outputs =
    {
      nixpkgs,
      nixpkgs-staging,
      devshells,
      ...
    }:
    let
      system = "aarch64-darwin";
      pkgs = nixpkgs.legacyPackages.${system};
      # I adapted this line from some Claude Sonnet 5 output; I don't
      # understand why this syntax is so different from the line above
      pkgs-staging = import nixpkgs-staging { inherit system; };
      # mkClangDevShell creates a devshell with things like clang 22,
      # clangd, etc., and it ensure the local Mac environment is
      # set up for C++ development
      mkClangDevShell = import "${devshells}/lib/mk-clang-dev-shell.nix" { inherit pkgs; };
    in
    {
      devShells.${system} = {
        # default development shell with Clang 22
        # launch with `nix develop`
        default = mkClangDevShell {
          extraPackages = with pkgs; [
            pkgs-staging.cmake
            gersemi
            lldb
            ninja
          ];
        };
      };
    };
}
