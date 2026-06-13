#!/usr/bin/env python3
"""Install a PS3 PKG or game folder to emulator/real hardware paths."""

from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path


def install_pkg(pkg_path: Path, emulator_root: Path) -> int:
    if not pkg_path.is_file():
        print(f"error: PKG not found: {pkg_path}", file=sys.stderr)
        return 1

    pkg_dir = emulator_root / "dev_hdd0" / "game" / "PKG"
    pkg_dir.mkdir(parents=True, exist_ok=True)
    dst = pkg_dir / pkg_path.name
    shutil.copy2(pkg_path, dst)
    print(f"Copied PKG to {dst}")
    print("Install via RPCS3: File -> Install PKG")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Install PS3 PKG to RPCS3 virtual HDD")
    parser.add_argument("pkg", type=Path, help="Path to .pkg file")
    parser.add_argument(
        "--emulator-root",
        type=Path,
        default=Path(__file__).resolve().parents[4] / "ps3_emulator",
        help="RPCS3 user data root",
    )
    args = parser.parse_args()
    return install_pkg(args.pkg, args.emulator_root)


if __name__ == "__main__":
    raise SystemExit(main())
