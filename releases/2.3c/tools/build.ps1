# Build a Tone+FX custom OS from the user's own stock Digitone .syx plus the
# suite elemod and the required core. No firmware is distributed with this repo.
#
#   ./tools/build.ps1 -Stock Digitone_and_Digitone_Keys_OS1.43.syx `
#                     -Out ToneFX.syx -Version 2.3c [-Core core-dn1-2.0a.elemod]
#
# One file is the whole suite (tonefx-2.3c.elemod), but it requires the Digitone
# core (core-dn1-2.0a.elemod), which elekloader provides. -Core defaults to a
# core next to this repo; pass it if it is elsewhere.
#
# Needs elekloader importable (installed, or PYTHONPATH set to its folder).
param(
    [Parameter(Mandatory = $true)][string]$Stock,
    [string]$Out = "ToneFX.syx",
    [string]$Version = "2.3c",
    [string]$Core = ""
)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
if (-not $Core) {
    foreach ($c in @("$root\elemods\core-dn1-2.0a.elemod", "$root\core-dn1-2.0a.elemod")) {
        if (Test-Path -LiteralPath $c) { $Core = $c; break }
    }
}
if (-not $Core -or -not (Test-Path -LiteralPath $Core)) {
    throw "the Digitone core (core-dn1-2.0a.elemod) is required; pass -Core <path>"
}
$suite = Join-Path $root "elemods\tonefx-2.3c.elemod"
if (-not (Test-Path -LiteralPath $suite)) { throw "missing $suite" }
$argv = @("-m", "elekloader.patch", "--stock", $Stock,
          "--mod", $Core, "--mod", $suite, "--out", $Out, "--version", $Version)
Write-Host "python $($argv -join ' ')"
& python @argv
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
