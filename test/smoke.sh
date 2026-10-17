#!/usr/bin/env bash
# Smoke test for mdldec (POSIX build output).
# POSIX counterpart of test/smoke.ps1; used by the Nix flake's check.
# Usage: test/smoke.sh [-Exe <path>] [-WorkDir <path>]
set -u

Exe="bin/mdldec"
WorkDir=""

while [ $# -gt 0 ]; do
  case "$1" in
    -Exe) Exe="$2"; shift 2 ;;
    -WorkDir) WorkDir="$2"; shift 2 ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done

root=$(cd "$(dirname "$0")/.." && pwd)
case "$Exe" in
  /*) ;;
  *) Exe="$root/$Exe" ;;
esac

[ -x "$Exe" ] || { echo "FAIL: exe not found or not executable: $Exe"; exit 1; }
echo "Exe: $Exe"

if [ -z "$WorkDir" ]; then
  WorkDir=$(mktemp -d "${TMPDIR:-/tmp}/mdldec-test-XXXXXXXX")
fi
mkdir -p "$WorkDir"
echo "WorkDir: $WorkDir"

cleanup() { rm -rf "$WorkDir"; }
trap cleanup EXIT

# The committed synthetic model stands in for testdata/gen.py: the Nix build
# sandbox has no python, and the fixture is deterministic either way.
mdl1="$WorkDir/synthetic.mdl"
mdl2="$WorkDir/synthetic2.mdl"
cp "$root/testdata/synthetic.mdl" "$mdl1" || { echo "FAIL: fixture missing"; exit 1; }
cp "$mdl1" "$mdl2"

fail() { echo "FAIL: $1"; exit 1; }
ok() { echo "PASS: $1"; }

# 1. Single-file decompile, output must land next to input.
"$Exe" -nopause "$mdl1" >/dev/null || fail "single decompile exit=$?"
qc1="$WorkDir/synthetic/synthetic.qc"
bmp1="$WorkDir/synthetic/maps_8bit/testtex.bmp"
[ -f "$qc1" ] || fail "missing $qc1"
[ -f "$bmp1" ] || fail "missing $bmp1"
grep -q '\$modelname' "$qc1" || fail "qc missing \$modelname"
grep -q '\$cdtexture' "$qc1" || fail "qc missing \$cdtexture"
ok "single-file decompile"

# 2. Multi-file drag-and-drop mode.
rm -rf "$WorkDir/synthetic" "$WorkDir/synthetic2"
"$Exe" -nopause "$mdl1" "$mdl2" >/dev/null || fail "multi decompile exit=$?"
[ -f "$qc1" ] || fail "missing $qc1 (multi)"
qc2="$WorkDir/synthetic2/synthetic2.qc"
[ -f "$qc2" ] || fail "missing $qc2 (multi)"
ok "multi-file decompile"

# 3. -info mode.
info=$("$Exe" -nopause -info "$mdl1") || fail "-info exit=$?"
printf '%s' "$info" | grep -q 'Version: 10' || fail "-info content unexpected: $info"
ok "-info mode"

# 4. -help mode.
"$Exe" -help >/dev/null || fail "-help exit=$?"
ok "-help mode"

# 5. Invalid input must fail non-zero (and not hang: -nopause).
echo "not a model" > "$WorkDir/bad.mdl"
"$Exe" -nopause "$WorkDir/bad.mdl" >/dev/null 2>&1
[ $? -ne 0 ] || fail "invalid input should fail non-zero"
ok "invalid input fails cleanly"

echo "ALL SMOKE TESTS PASSED"