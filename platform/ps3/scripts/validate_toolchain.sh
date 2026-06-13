#!/usr/bin/env bash
set -euo pipefail

export PS3DEV="${PS3DEV:-/usr/local/ps3dev}"
export PSL1GHT="${PSL1GHT:-$PS3DEV}"
export PATH="$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin"

ppu-gcc --version | head -1
test -f "$PSL1GHT/ppu/lib/lv2.ld"
echo "lv2.ld OK"
