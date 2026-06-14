#!/usr/bin/env python3
"""Validate RPCS3 boot settings and log milestones for Blazium PS3 homebrew."""

from __future__ import annotations

import argparse
import sys
import time
from pathlib import Path


def _read_tail(path: Path, max_bytes: int = 256_000) -> str:
    if not path.is_file():
        return ""
    data = path.read_bytes()
    if len(data) > max_bytes:
        data = data[-max_bytes:]
    return data.decode("utf-8", errors="replace")


def validate(log_path: Path, wait_seconds: int) -> int:
    print("RPCS3 boot validation")
    print(f"Log: {log_path}")
    print("")
    print("TTY checklist (RPCS3 Debug Console / TTY tab — not RPCS3.log):")
    print("  [ ] PS3: modules loaded")
    print("  [ ] PS3: NoRSX initialized.")
    print("  [ ] PS3: NoRSX first frame presented.")
    print("  [ ] PS3: Main::setup begin/end")
    print("  [ ] PS3: Main::start")
    print("  [ ] PS3: first main_loop_iterate")
    print("  [ ] PS3 Test: main scene loaded")
    print("")
    print("Launch from the RPCS3 game list (dev_hdd0/game/BLAZ00001), not Boot Game on export folder.")
    print("")

    deadline = time.time() + wait_seconds
    boot_ok = False

    while time.time() < deadline:
        text = _read_tail(log_path)
        boot_ok = "GUI: Boot successful" in text
        if boot_ok:
            break
        time.sleep(2)

    text = _read_tail(log_path)
    boot_ok = "GUI: Boot successful" in text
    llvm_precompile = "LLVM Precompilation: true" in text
    interpreter = "PPU Decoder: Interpreter (static)" in text or "Interpreter (static)" in text
    jump_warnings = text.count("Jump table not found")
    empty_title_id = "empty title_id" in text or "title_id=''" in text
    missing_db_config = "Cannot retrieve database config for empty title_id" in text
    boot_from_export = "exported_game/ps3_export" in text or "ps3_export/BLAZ00001" in text

    print("RPCS3.log results:")
    print(f"  Boot successful: {'yes' if boot_ok else 'no'}")
    print(f"  Interpreter (static): {'yes' if interpreter else 'no'}")
    print(f"  LLVM precompilation enabled: {'yes (bad)' if llvm_precompile else 'no (good)'}")
    print(f"  Jump table warnings: {jump_warnings}")
    print(f"  Empty title_id detected: {'yes (bad)' if empty_title_id else 'no (good)'}")
    print(f"  Missing database config: {'yes (bad)' if missing_db_config else 'no (good)'}")
    print(f"  Booted from export folder: {'yes (use game list instead)' if boot_from_export else 'no'}")
    print("")

    ok = boot_ok and not llvm_precompile and not missing_db_config
    if jump_warnings > 50:
        print(f"WARN: {jump_warnings} jump-table warnings — rebuild with rpcs3_compat=yes (no -ffunction-sections).")
    if boot_from_export or empty_title_id:
        print("WARN: Launch BLAZ00001 from RPCS3 game list after deploy_rpcs3.py.")

    if ok:
        print("PASS: RPCS3 boot looks healthy. Check TTY tab for PS3 milestones and visible UI.")
        return 0

    print("FAIL: RPCS3 boot validation failed.")
    if not boot_ok:
        print("  - Wait longer or confirm RPCS3 started the game.")
    if llvm_precompile:
        print("  - Disable LLVM Precompilation in RPCS3 config.")
    if missing_db_config:
        print("  - Deploy to dev_hdd0 and launch from game list so TITLE_ID config applies.")
    return 1


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate RPCS3 boot log for Blazium PS3")
    parser.add_argument(
        "--log",
        type=Path,
        default=Path(__file__).resolve().parents[4] / "ps3_emulator" / "log" / "RPCS3.log",
        help="Path to RPCS3.log",
    )
    parser.add_argument("--wait", type=int, default=30, help="Seconds to wait for Boot successful")
    args = parser.parse_args()
    return validate(args.log, args.wait)


if __name__ == "__main__":
    raise SystemExit(main())
