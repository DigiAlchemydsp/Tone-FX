#!/bin/sh
# Build a Tone+FX custom OS from the user's own stock Digitone .syx plus the
# packaged elemods. No firmware is distributed with this repo.
#
#   sh tools/build.sh Digitone_and_Digitone_Keys_OS1.43.syx ToneFX.syx 2.1h
#
# Needs elekloader importable (installed, or PYTHONPATH set to its folder).
set -e
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
STOCK=${1:?usage: build.sh STOCK.syx [OUT.syx] [VERSION]}
OUT=${2:-ToneFX.syx}
VERSION=${3:-2.1h}
MODS="core-dn1-2.0a digimeter-1.1 digieq-1.0 digiring-1.0 digifold-1.0 digifilter-1.0 digictl-1.3"
set -- --stock "$STOCK"
for m in $MODS; do
    set -- "$@" --mod "$ROOT/elemods/$m.elemod"
done
set -- "$@" --out "$OUT" --version "$VERSION"
python -m elekloader.patch "$@"
