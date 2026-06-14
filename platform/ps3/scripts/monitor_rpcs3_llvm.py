#!/usr/bin/env python3
"""Monitor RPCS3 PPU LLVM compile progress from RPCS3.log."""

from __future__ import annotations

import argparse
import sys
import time
from pathlib import Path

_SCRIPTS_DIR = Path(__file__).resolve().parent
if str(_SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(_SCRIPTS_DIR))

from rpcs3_llvm_progress import format_llvm_status, parse_llvm_progress, read_log_text


def monitor(log_path: Path, interval: int, once: bool) -> int:
    print(f"Monitoring LLVM progress: {log_path}")
    print(f"Poll interval: {interval}s")
    print("")

    while True:
        text = read_log_text(log_path)
        progress = parse_llvm_progress(text)
        print(f"--- {time.strftime('%H:%M:%S')} ---")
        for line in format_llvm_status(progress):
            print(line)
        print("")

        if once:
            return 0
        if progress.loaded:
            print("LLVM loaded — game should be starting. Check TTY for PS3: early_init.")
            return 0
        if progress.mega_module_active:
            print("Mega-module active — wait 5-10 min even if CPU looks idle.")
        elif progress.in_progress:
            print("LLVM compile in progress — first boot can take 30-40 min total.")

        time.sleep(interval)


def main() -> int:
    parser = argparse.ArgumentParser(description="Monitor RPCS3 LLVM compile progress")
    parser.add_argument(
        "--log",
        type=Path,
        default=Path(__file__).resolve().parents[4] / "ps3_emulator" / "log" / "RPCS3.log",
        help="Path to RPCS3.log",
    )
    parser.add_argument("--interval", type=int, default=30, help="Poll interval in seconds")
    parser.add_argument("--once", action="store_true", help="Print status once and exit")
    args = parser.parse_args()
    return monitor(args.log, args.interval, args.once)


if __name__ == "__main__":
    raise SystemExit(main())
