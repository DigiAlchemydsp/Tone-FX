#!/bin/sh
# Build a Tone+FX 4.3b custom OS from your own stock .syx plus the suite and core.
# No firmware is distributed here.
#
#   ./tools/build.sh Digitone_and_Digitone_Keys_OS1.44.syx ToneFX.syx
#   ./tools/build.sh Digitakt_OS1.54.syx ToneFX-DT.syx
set -eu
if [ "$#" -lt 1 ]; then
    echo "usage: $0 STOCK.syx [OUT.syx]" >&2
    exit 2
fi
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
stock=$1
out=${2:-ToneFX.syx}
case "$stock" in
    *Digitakt*) suite=tonefx-4.3b-os1.54.elemod; core=core-4.0.1-os1.54.elemod ;;
    *)          suite=tonefx-4.3b-os1.44.elemod; core=core-dn1-4.1-os1.44.elemod ;;
esac
exec python -m elekloader.patch --stock "$stock" \
    --mod "$root/elemods/$core" --mod "$root/elemods/$suite" \
    --out "$out" --version 4.3b
