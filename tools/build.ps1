# Build a Tone+FX custom OS from the user's own stock Digitone .syx plus the
# packaged elemods. No firmware is distributed with this repo.
#
#   ./tools/build.ps1 -Stock Digitone_and_Digitone_Keys_OS1.43.syx `
#                     -Out ToneFX.syx -Version 2.1h
#
# Needs elekloader importable (installed, or PYTHONPATH set to its folder).
param(
    [Parameter(Mandatory = $true)][string]$Stock,
    [string]$Out = "ToneFX.syx",
    [string]$Version = "2.1h"
)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$mods = @(
    "core-dn1-2.0a", "digimeter-1.1", "digieq-1.0", "digiring-1.0",
    "digifold-1.0", "digifilter-1.0", "digictl-1.3"
)
$argv = @("-m", "elekloader.patch", "--stock", $Stock)
foreach ($m in $mods) {
    $p = Join-Path $root "elemods\$m.elemod"
    if (-not (Test-Path -LiteralPath $p)) { throw "missing $p" }
    $argv += @("--mod", $p)
}
$argv += @("--out", $Out, "--version", $Version)
Write-Host "python $($argv -join ' ')"
& python @argv
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
