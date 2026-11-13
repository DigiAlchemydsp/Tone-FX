# Build a Tone+FX 4.3b custom OS from your own stock .syx plus the suite and core.
# No firmware is distributed here.
#
#   ./tools/build.ps1 -Stock Digitone_and_Digitone_Keys_OS1.44.syx -Out ToneFX.syx
#   ./tools/build.ps1 -Stock Digitakt_OS1.54.syx -Out ToneFX-DT.syx
#
# The device is detected from the stock file name. The matching elemod and core
# are taken from ../elemods; pass -Core or -Suite to override.
param(
    [Parameter(Mandatory = $true)][string]$Stock,
    [string]$Out = "ToneFX.syx",
    [string]$Version = "4.3b",
    [string]$Core = "",
    [string]$Suite = ""
)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$isDT = $Stock -match "Digitakt"
if (-not $Suite) {
    $name = if ($isDT) { "tonefx-4.3b-os1.54.elemod" } else { "tonefx-4.3b-os1.44.elemod" }
    $Suite = Join-Path $root "elemods\$name"
}
if (-not $Core) {
    $coreName = if ($isDT) { "core-4.0.1-os1.54.elemod" } else { "core-dn1-4.1-os1.44.elemod" }
    $Core = Join-Path $root "elemods\$coreName"
}
if (-not (Test-Path -LiteralPath $Suite)) { throw "missing $Suite" }
if (-not (Test-Path -LiteralPath $Core)) { throw "missing $Core" }
$argv = @("-m", "elekloader.patch", "--stock", $Stock,
          "--mod", $Core, "--mod", $Suite, "--out", $Out, "--version", $Version)
Write-Host "python $($argv -join ' ')"
& python @argv
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
