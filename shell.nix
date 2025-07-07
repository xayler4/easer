{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
	buildInputs = [
		pkgs.clang
		pkgs.cmake
		pkgs.gdb
		pkgs.boost
	];
	
	# shellHook = ''
	# '';
}
