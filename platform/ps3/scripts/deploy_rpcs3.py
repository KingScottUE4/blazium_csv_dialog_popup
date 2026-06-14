#!/usr/bin/env python3
"""Deploy a PS3 export folder to the RPCS3 emulator data tree."""

from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path


def deploy(export_dir: Path, title_id: str, emulator_root: Path) -> int:
    src = export_dir / title_id
    if not src.is_dir():
        print(f"error: export directory not found: {src}", file=sys.stderr)
        return 1

    dst = emulator_root / "dev_hdd0" / "game" / title_id
    dst.parent.mkdir(parents=True, exist_ok=True)

    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst)

    config_src = Path(__file__).resolve().parent / "rpcs3_game_config.yml"
    if config_src.is_file():
        config_dst = emulator_root / "config" / f"{title_id}.yml"
        config_dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(config_src, config_dst)
        print(f"Installed RPCS3 config {config_dst}")

    print(f"Deployed {src} -> {dst}")
    print("Launch RPCS3 with PPU Interpreter (static) and Debug Console Mode for TTY output.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Deploy PS3 export to RPCS3 dev_hdd0")
    parser.add_argument("--export-dir", required=True, type=Path, help="Export output directory containing TITLE_ID folder")
    parser.add_argument("--title-id", default="BLAZ00001", help="PS3 TITLE_ID")
    parser.add_argument(
        "--emulator-root",
        type=Path,
        default=Path(__file__).resolve().parents[4] / "ps3_emulator",
        help="RPCS3 user data root (contains dev_hdd0/)",
    )
    args = parser.parse_args()
    return deploy(args.export_dir, args.title_id, args.emulator_root)


if __name__ == "__main__":
    raise SystemExit(main())
