# mdldec — GoldSrc model decompiler (MSVC-only CLI)

[![ci](https://github.com/nenquen/mdldec/actions/workflows/ci.yml/badge.svg)](https://github.com/nenquen/mdldec/actions/workflows/ci.yml)

`mdldec` decompiles GoldSrc (Half-Life 1) formats:

* `.mdl` → `.qc` + `.smd` + `.bmp` textures
* `.spr` → `.qc` + `.bmp` frames
* `.wad` / `.bsp` (embedded textures) → `.bmp`

Based on `DecompMDL` from [Toodles2You/halflife-tools](https://github.com/Toodles2You/halflife-tools).
See `LICENSE` (Valve HL1 SDK license applies to the core).

## Build — MSVC only, no MinGW

Open `mdldec.sln` in Visual Studio and build, or from CLI:

```bat
msbuild mdldec.sln /p:Configuration=Release /p:Platform=x64
```

Output: `bin\x64\Release\mdldec.exe`

Alternative via CMake (Visual Studio generator only):

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Tests

```powershell
powershell -ExecutionPolicy Bypass -File test/smoke.ps1 -Exe bin/x64/Release/mdldec.exe
```

Or via CMake: `ctest --test-dir build -C Release --output-on-failure`.
CI (`.github/workflows/ci.yml`) builds Debug+Release with MSBuild and
Release with CMake, then runs the smoke test.

## Usage — drag-and-drop

Easiest: drag one or more `.mdl` files onto `mdldec.exe`.
Output goes next to each input: `<input-dir>\<model>\<model>.qc`.

```bat
mdldec <input> [<output>]
mdldec a.mdl b.mdl c.mdl
mdldec player.mdl out_dir
mdldec player.mdl out.qc
mdldec -info model.mdl
mdldec -info acts,events,bodygroups model.mdl
```

Options: `-cd`, `-cdtexture`, `-cdanim`, `-pattern`, `-info`, `-pause`, `-nopause`.

Drag-and-drop runs pause automatically so the window stays open.
Scripts should pass `-nopause`.
