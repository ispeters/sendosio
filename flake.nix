{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-26.05-darwin";
  # nixpkgs-unstable has CMake 4.1.something, but our CMakeLists.txt
  # (optionally) uses features of CMake 4.3, so pull in the staging-next
  # channel, which already has a CMake 4.3.4 package.
  inputs.nixpkgs-staging.url = "github:NixOS/nixpkgs/staging-next";
  outputs = { self, nixpkgs, nixpkgs-staging }:
    let
      system = "aarch64-darwin";
      pkgs = nixpkgs.legacyPackages.${system};
      # I adapted this line from some Claude Sonnet 5 output; I don't
      # understand why this syntax is so different from the line above
      pkgs-staging = import nixpkgs-staging { inherit system; };
    in
    {
      devShells.${system} = {
        # default development shell with Clang 22
        # launch with `nix develop`
        default = pkgs.mkShell.override {
          stdenv = pkgs.llvmPackages_22.libcxxStdenv;
        } {
          packages = with pkgs; [
            clang-tools
            pkgs-staging.cmake
            gersemi
            ninja
          ];

          shellHook = ''
            export NIX_CFLAGS_COMPILE="$NIX_CFLAGS_COMPILE -B${pkgs.llvmPackages_22.libcxx}/lib"
          '';
        };
      };
    };
}
