#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BLAZIUM_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
# shellcheck source=setup_ps3dev_env.sh
source "$SCRIPT_DIR/setup_ps3dev_env.sh"

echo "== PS3 smoke test =="
bash "$SCRIPT_DIR/validate_toolchain.sh"

cd "$BLAZIUM_ROOT"
echo "== Building PS3 release template =="
scons platform=ps3 target=template_release arch=ppc64 module_ps3_module_enabled=yes -j"$(nproc)"

echo "== Installing export templates =="
python3 "$SCRIPT_DIR/install_templates.py" --blazium-root "$BLAZIUM_ROOT"

TEST_ROOT="/mnt/d/projects/console_support/ps3_test"
if [ -d "$TEST_ROOT/export/ps3" ]; then
    echo "== Deploying test export to RPCS3 =="
    python3 "$SCRIPT_DIR/deploy_rpcs3.py" \
        --export-dir "$TEST_ROOT/export/ps3" \
        --title-id BLAZ00001 \
        --emulator-root "/mnt/d/projects/console_support/ps3_emulator"
else
    echo "note: export ps3_test first (Project -> Export -> PS3), then re-run smoke_test.sh"
fi

echo ""
echo "RPCS3 validation:"
echo "  1. Enable Debug Console Mode in RPCS3"
echo "  2. Launch dev_hdd0/game/BLAZ00001/USRDIR/EBOOT.BIN"
echo "  3. Confirm TTY shows PS3 boot logs and pack loading"
