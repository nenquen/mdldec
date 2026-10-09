# Smoke test for mdldec (MSVC build output).
# Usage: powershell -ExecutionPolicy Bypass -File test/smoke.ps1 [-Exe <path>] [-WorkDir <path>]
param(
  [string]$Exe = "bin/Release/mdldec.exe",
  [string]$WorkDir = ""
)

$ErrorActionPreference = "Stop"

function Fail($msg) { Write-Host "FAIL: $msg"; exit 1 }
function Ok($msg) { Write-Host "PASS: $msg" }

$root = Split-Path -Parent $PSScriptRoot
if (-not [IO.Path]::IsPathRooted($Exe)) { $Exe = Join-Path $root $Exe }
if (-not (Test-Path $Exe)) { Fail "exe not found: $Exe" }
Write-Host "Exe: $Exe"

# Python discovery: prefer 'python', fall back to the 'py -3' launcher
# (bare 'python' can resolve to the Microsoft Store stub on some machines).
$script:PyExe = $null
$script:PyArgs = @()
foreach ($cand in @(@{ exe = 'python'; args = @() }, @{ exe = 'py'; args = @('-3') })) {
  try {
    & $cand.exe $cand.args --version
    if ($LASTEXITCODE -eq 0) {
      $script:PyExe = $cand.exe
      $script:PyArgs = $cand.args
      break
    }
  } catch { }
}
if (-not $script:PyExe) { Fail "no working python found (tried 'python', 'py -3'). PATH=$env:PATH" }

if ($WorkDir -eq "") {
  $WorkDir = Join-Path ([IO.Path]::GetTempPath()) ("mdldec-test-" + [Guid]::NewGuid().ToString("N"))
}
New-Item -ItemType Directory -Force -Path $WorkDir | Out-Null
Write-Host "WorkDir: $WorkDir"

try {
  $gen = Join-Path $root "testdata/gen.py"
  $mdl1 = Join-Path $WorkDir "synthetic.mdl"
  $mdl2 = Join-Path $WorkDir "synthetic2.mdl"

  & $script:PyExe $script:PyArgs $gen $mdl1
  if ($LASTEXITCODE -ne 0) { Fail "gen.py failed" }
  Copy-Item $mdl1 $mdl2

  # 1. Single-file decompile, output must land next to input.
  & $Exe -nopause $mdl1
  if ($LASTEXITCODE -ne 0) { Fail "single decompile exit=$LASTEXITCODE" }
  $qc1 = Join-Path $WorkDir "synthetic/synthetic.qc"
  $bmp1 = Join-Path $WorkDir "synthetic/maps_8bit/testtex.bmp"
  if (-not (Test-Path $qc1)) { Fail "missing $qc1" }
  if (-not (Test-Path $bmp1)) { Fail "missing $bmp1" }
  $qcText = Get-Content $qc1 -Raw
  if ($qcText -notmatch '\$modelname' -or $qcText -notmatch '\$cdtexture') { Fail "qc content unexpected" }
  Ok "single-file decompile"

  # 2. Multi-file drag-and-drop mode.
  Remove-Item -Recurse -Force (Join-Path $WorkDir "synthetic"), (Join-Path $WorkDir "synthetic2") -ErrorAction SilentlyContinue
  & $Exe -nopause $mdl1 $mdl2
  if ($LASTEXITCODE -ne 0) { Fail "multi decompile exit=$LASTEXITCODE" }
  if (-not (Test-Path $qc1)) { Fail "missing $qc1 (multi)" }
  $qc2 = Join-Path $WorkDir "synthetic2/synthetic2.qc"
  if (-not (Test-Path $qc2)) { Fail "missing $qc2 (multi)" }
  Ok "multi-file decompile"

  # 3. -info mode.
  $info = & $Exe -nopause -info $mdl1
  if ($LASTEXITCODE -ne 0) { Fail "-info exit=$LASTEXITCODE" }
  if (($info -join "`n") -notmatch "Version: 10") { Fail "-info content unexpected: $info" }
  Ok "-info mode"

  # 4. -help mode.
  & $Exe -help | Out-Null
  if ($LASTEXITCODE -ne 0) { Fail "-help exit=$LASTEXITCODE" }
  Ok "-help mode"

  # 5. Invalid input must fail non-zero (and not hang: -nopause).
  $bad = Join-Path $WorkDir "bad.mdl"
  [IO.File]::WriteAllText($bad, "not a model")
  & $Exe -nopause $bad | Out-Null
  if ($LASTEXITCODE -eq 0) { Fail "invalid input should fail non-zero" }
  Ok "invalid input fails cleanly"

  Write-Host "ALL SMOKE TESTS PASSED"
}
finally {
  Remove-Item -Recurse -Force $WorkDir -ErrorAction SilentlyContinue
}
