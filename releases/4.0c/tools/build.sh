#!/bin/sh
# Build a Tone+FX custom OS from the user's own stock Digitone .syx plus the
# suite elemod and the required core. No firmware is distributed with this repo.
#
#   sh tools/build.sh STOCK.syx [OUT.syx] [VERSION] [CORE.elemod] [SUITE.elemod]
#
# One file is the whole suite, but it requires the Digitone core
# (core-dn1-4.0.elemod), which elekloader provides. The 4th argument is the core
# path; without it, a core next to this repo is used. The suite is picked from
# the stock name: tonefx-4.0a-os1.44.elemod for OS 1.44, else tonefx-4.0a.
#
# Needs elekloader importable (installed, or PYTHONPATH set to its folder).
set -e
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
STOCK=${1:?usage: build.sh STOCK.syx [OUT.syx] [VERSION] [CORE.elemod] [SUITE.elemod]}
OUT=${2:-ToneFX.syx}
VERSION=${3:-4.0a}
CORE=${4:-}
SUITE=${5:-}
if [ -z "$CORE" ]; then
    for c in "$ROOT/elemods/core-dn1-4.0.elemod" "$ROOT/core-dn1-4.0.elemod"; do
        if [ -f "$c" ]; then CORE=$c; break; fi
    done
fi
if [ -z "$CORE" ] || [ ! -f "$CORE" ]; then
    echo "the Digitone core (core-dn1-4.0.elemod) is required; pass its path" >&2
    exit 1
fi
if [ -z "$SUITE" ]; then
    case "$STOCK" in
        *1.44*) SUITE="$ROOT/elemods/tonefx-4.0a-os1.44.elemod" ;;
        *)      SUITE="$ROOT/elemods/tonefx-4.0a.elemod" ;;
    esac
fi
python -m elekloader.patch --stock "$STOCK" --mod "$CORE" --mod "$SUITE" \
    --out "$OUT" --version "$VERSION"
