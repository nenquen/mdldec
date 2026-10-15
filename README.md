# mdldec

Based on DecompMDL from https://github.com/Toodles2You/halflife-tools

## Building

### Windows (MSVC, reference build)

```bat
build.bat Release x64
```

Output goes to `bin\Release\mdldec.exe`. `build.bat` uses `cl.exe` directly —
no MSBuild, no CMake, no MinGW.

### Linux / NixOS (flake)

```bash
nix build            # -> ./result/bin/mdldec
nix run . -- -help
```

The Half-Life 1 SDK license is not OSI-approved, so the package is marked
`unfree`. nixpkgs refuses to evaluate unfree packages by default:

```bash
NIXPKGS_ALLOW_UNFREE=1 nix build --impure
```

The `--impure` is only needed for that environment variable. If your nixpkgs
already sets `allowUnfree = true` (most home-manager configs do), plain
`nix build` is enough.

A dev shell with the build dependencies plus gdb and valgrind:

```bash
nix develop
```

### Linux (without Nix)

```bash
cc -std=c11 -O2 -Isrc src/*.c -o mdldec -lm
```

CMake is not used on Linux: `CMakeLists.txt` is deliberately MSVC-only and
hard-fails on any other compiler.

## Portability notes

The source builds with MSVC on Windows and GCC/Clang on POSIX. Two spots in
`src/pch.h` carry the difference:

- `strcasecmp`/`strdup` map to `_stricmp`/`_strdup` under `_WIN32`, and come
  from `<strings.h>` elsewhere. `_DEFAULT_SOURCE` is defined at the top of
  `pch.h` so glibc exposes them under `-std=c11`.
- `common.c` uses the POSIX two-argument `mkdir(path, 0777)` on GCC/Clang and
  MSVC's one-argument `_mkdir(path)` on Windows.

The `winmm` library is linked on MSVC but nothing in the source calls into it;
it is inherited from DecompMDL and is not linked on POSIX.

## Tests

`test/smoke.sh` (POSIX) and `test/smoke.ps1` (Windows) cover the same five
scenarios: single-file decompile, multi-file decompile, `-info`, `-help`, and
a non-zero exit on invalid input. The Nix build runs the POSIX one as its
`checkPhase`, so `nix build` exercises the binary rather than just the compiler.

## License

Half Life 1 SDK license — see `LICENSE`.