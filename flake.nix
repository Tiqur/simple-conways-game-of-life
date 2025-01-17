{
  inputs.nixpkgs.url = "github:nixos/nixpkgs";

  outputs = { self, nixpkgs }:
  let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in {
    devShells.x86_64-linux.default = pkgs.mkShell {
      buildInputs = with pkgs; [  
        # Build / dev tools
        cmake
        clang
        clang-tools
        valgrind
        gdb
        pkg-config

        # Dependencies
        glew
        glfw3
      ];
    };
  };
}

