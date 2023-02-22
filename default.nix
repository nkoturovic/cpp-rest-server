{
  system ? builtins.currentSystem,
  lock ? builtins.fromJSON (builtins.readFile ./flake.lock),
   kotur-nixpkgs ? let
     koturPkgs = fetchTarball {
        url = "https://github.com/nkoturovic/kotur-nixpkgs/archive/${lock.nodes.koturNixPkgs.locked.rev}.tar.gz";
        sha256 = lock.nodes.koturNixPkgs.locked.narHash;
     }; in import koturPkgs {
        inherit system;
     },
  pkgs ? let
    nixpkgs = fetchTarball {
      url = "https://github.com/NixOS/nixpkgs/archive/${lock.nodes.nixpkgs.locked.rev}.tar.gz";
      sha256 = lock.nodes.nixpkgs.locked.narHash;
    };
     in
    import nixpkgs {
      overlays = [];
      config = {};
      inherit system;
    },
  miniCompileCommands ?
    fetchTarball {
      url = "https://github.com/danielbarter/mini_compile_commands/archive/${lock.nodes.miniCompileCommands.locked.rev}.tar.gz";
      sha256 = lock.nodes.miniCompileCommands.locked.narHash;
    },
}: let
  package = pkgs.gccStdenv.mkDerivation (self: {
    name = "cpp-rest-server";
    version = "0.0.3";

    nativeBuildInputs = with pkgs; [
      gccStdenv # Also used bellow with mini_compile_commands in shell
      cowsay
      ncurses
      cmake
      gnumake
    ];

    buildInputs = with pkgs; [
        fmt
        sqlite
        soci
        boost174
        nlohmann_json
        spdlog
        http-parser
        restinio
        asio
        kotur-nixpkgs.cpp-jwt
        openssl
    ];

    src = builtins.path {
      path = ./.;
      filter = path: type:
        !(pkgs.lib.hasPrefix "." (baseNameOf path));
    };

    buildDir = "build-nix-${self.name}-${self.version}";

    configurePhase = ''
      mkdir ./${self.buildDir} && cd ./${self.buildDir}
      cmake .. -DCMAKE_BUILD_TYPE=Release -DCPP_REST_SERVER_BUILD_EXAMPLES=OFF
    '';

    buildPhase = ''
      make -j$(nproc)
    '';

    installPhase = ''
      mkdir -p $out/bin
      cp ${self.name} $out/bin/
    '';

    passthru = {
      inherit pkgs shell;
    };
  });

  # Shell specific parameters

  # Using mini_compile_commands to export compile_commands.json
  # https://github.com/danielbarter/mini_compile_commands/
  mcc-env = (pkgs.callPackage miniCompileCommands {}).wrap pkgs.gccStdenv;

  # Development shell
  shell = (pkgs.mkShell.override {stdenv = mcc-env;}) {
    inputsFrom = [
      package
    ];

    packages = with pkgs; [
      neovim
    ];

    shellHook = ''
      PS1="\[\e[32m\][\[\e[m\]\[\e[33m\]nix-shell\\[\e[m\]:\[\e[36m\]\w\[\e[m\]\[\e[32m\]]\[\e[m\]\\$\[\e[m\] "
      alias ll="ls -l"
      cowsay "Welcome to the '${package.name}' dev environment!" 2> /dev/null
    '';
  };
in
  package
