#!/bin/sh
# Build a Tone+FX custom OS from the user's own stock Digitone .syx plus the
# suite elemod and the required core. No firmware is distributed with this repo.
#
#   sh tools/build.sh Digitone_and_Digitone_Keys_OS1.43.syx ToneFX.syx 2.3a [core.elemod]
#
# One file is the whole suite (tonefx-2.3a.elemod), but it requires the Digitone
# core (core-dn1-2.0a.elemod), which elekloader provides. The 4th argument is
# the core path; without it, a core next to this repo is used.
#
# Needs elekloader importable (installed, or PYTHONPATH set to its folder).
set -e
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
STOCK=${1:?usage: build.sh STOCK.syx [OUT.syx] [VERSION] [CORE.elemod]}
OUT=${2:-ToneFX.syx}
VERSION=${3:-2.3a}
CORE=${4:-}
if [ -z "$CORE" ]; then
    for c in "$ROOT/elemods/core-dn1-2.0a.elemod" "$ROOT/core-dn1-2.0a.elemod"; do
        if [ -f "$c" ]; then CORE=$c; break; fi
    done
fi
if [ -z "$CORE" ] || [ ! -f "$CORE" ]; then
    echo "the Digitone core (core-dn1-2.0a.elemod) is required; pass its path" >&2
    exit 1
fi
SUITE="$ROOT/elemods/tonefx-2.3a.elemod"
python -m elekloader.patch --stock "$STOCK" --mod "$CORE" --mod "$SUITE" \
    --out "$OUT" --version "$VERSION"
