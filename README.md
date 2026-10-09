# mdldec — GoldSrc model decompiler (MSVC-only CLI)

[![ci](https://github.com/nenquen/mdldec/actions/workflows/ci.yml/badge.svg)](https://github.com/nenquen/mdldec/actions/workflows/ci.yml)

`mdldec` decompiles GoldSrc (Half-Life 1) formats:

* `.mdl` → `.qc` + `.smd` + `.bmp` textures
* `.spr` → `.qc` + `.bmp` frames
* `.wad` / `.bsp` (embedded textures) → `.bmp`

Based on `DecompMDL` from [Toodles2You/halflife-tools](https://github.com/Toodles2You/halflife-tools).
See `LICENSE` (Valve HL1 SDK license applies to the core).

## Build — pure MSVC only, no MSBuild, no MinGW

Double-click or run:

```bat
build.bat Release x64
```

Output: `bin\Release\mdldec.exe`. This calls `cl.exe` directly
(VS located via `vswhere`). `mdldec.sln` also exists for IDE users.

## Tests

```powershell
powershell -ExecutionPolicy Bypass -File test/smoke.ps1 -Exe bin/Release/mdldec.exe
```

Or via CMake: `ctest --test-dir build -C Release --output-on-failure`.
CI (`.github/workflows/ci.yml`) runs `build.bat` (Release+Debug) and
the smoke test on Release.

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
