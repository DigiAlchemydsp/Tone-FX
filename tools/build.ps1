# Build a Tone+FX custom OS from the user's own stock Digitone .syx plus the
# suite elemod and the required core. No firmware is distributed with this repo.
#
#   ./tools/build.ps1 -Stock Digitone_and_Digitone_Keys_OS1.43.syx `
#                     -Out ToneFX.syx -Version 3.5r [-Core core-dn1-4.0.elemod]
#
# One file is the whole suite, but it requires the Digitone core
# (core-dn1-4.0.elemod), which elekloader provides. -Core defaults to a core next
# to this repo; pass it if it is elsewhere. The suite is picked from the stock
# file: tonefx-3.5r-os1.44.elemod when -Stock names OS 1.44, else tonefx-3.5r.
#
# Needs elekloader importable (installed, or PYTHONPATH set to its folder).
param(
    [Parameter(Mandatory = $true)][string]$Stock,
    [string]$Out = "ToneFX.syx",
    [string]$Version = "3.5r",
    [string]$Core = "",
    [string]$Suite = ""
)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
if (-not $Core) {
    foreach ($c in @("$root\elemods\core-dn1-4.0.elemod", "$root\core-dn1-4.0.elemod")) {
        if (Test-Path -LiteralPath $c) { $Core = $c; break }
    }
}
if (-not $Core -or -not (Test-Path -LiteralPath $Core)) {
    throw "the Digitone core (core-dn1-4.0.elemod) is required; pass -Core <path>"
}
if (-not $Suite) {
    $name = if ($Stock -match "1\.44") { "tonefx-3.5r-os1.44.elemod" } else { "tonefx-3.5r.elemod" }
    $Suite = Join-Path $root "elemods\$name"
}
if (-not (Test-Path -LiteralPath $Suite)) { throw "missing $Suite" }
$argv = @("-m", "elekloader.patch", "--stock", $Stock,
          "--mod", $Core, "--mod", $Suite, "--out", $Out, "--version", $Version)
Write-Host "python $($argv -join ' ')"
& python @argv
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
