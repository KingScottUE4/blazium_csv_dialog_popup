#!/usr/bin/env python3
"""Parse RPCS3.log for PPU LLVM compile progress."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

MEGA_MODULE_THRESHOLD = 100_000
COMPILE_RE = re.compile(r"LLVM: Compiling module ")
COMPILED_RE = re.compile(r"LLVM: Compiled module ")
LOADED_RE = re.compile(r"LLVM: Loaded module ")
MEGA_RE = re.compile(r"LLVM: (\d+) functions generated \(code_size=0x[0-9a-f]+, num_func=(\d+)")
GHTAM_RE = re.compile(r"GHTAMzWrMaH4uqsLwAwr9h")


@dataclass
class LlvmProgress:
    compiling: int = 0
    compiled: int = 0
    loaded: bool = False
    mega_module_active: bool = False
    mega_module_seen: bool = False
    max_functions: int = 0
    last_compiling_line: str = ""

    @property
    def in_progress(self) -> bool:
        return self.compiling > 0 and not self.loaded


def parse_llvm_progress(log_text: str) -> LlvmProgress:
    progress = LlvmProgress()
    last_compile_line = ""

    for line in log_text.splitlines():
        if COMPILE_RE.search(line):
            progress.compiling += 1
            last_compile_line = line.strip()
        if COMPILED_RE.search(line):
            progress.compiled += 1
            if progress.compiling > 0:
                progress.compiling -= 1
        if LOADED_RE.search(line):
            progress.loaded = True
            progress.compiling = 0

        mega_match = MEGA_RE.search(line)
        if mega_match:
            num_func = int(mega_match.group(2))
            progress.max_functions = max(progress.max_functions, num_func)
            if num_func >= MEGA_MODULE_THRESHOLD:
                progress.mega_module_seen = True

        if GHTAM_RE.search(line) and COMPILE_RE.search(line):
            progress.mega_module_active = True

    if last_compile_line:
        progress.last_compiling_line = last_compile_line
        if GHTAM_RE.search(last_compile_line) and progress.compiling > 0:
            progress.mega_module_active = True

    return progress


def format_llvm_status(progress: LlvmProgress) -> list[str]:
    lines = [
        f"  LLVM modules compiled: {progress.compiled}",
        f"  LLVM modules in progress: {progress.compiling}",
        f"  LLVM loaded into main_thread: {'yes' if progress.loaded else 'no'}",
    ]
    if progress.max_functions > 0:
        lines.append(f"  LLVM max functions in one module: {progress.max_functions:,}")
    if progress.mega_module_active:
        lines.append("  LLVM mega-module active: yes (low CPU for 5-10 min is normal — do not close RPCS3)")
    elif progress.mega_module_seen:
        lines.append("  LLVM mega-module seen: yes (already compiled or loading)")
    if progress.last_compiling_line:
        short = progress.last_compiling_line
        if len(short) > 120:
            short = short[:117] + "..."
        lines.append(f"  LLVM last compile line: {short}")
    return lines


def read_log_text(log_path: Path, max_bytes: int = 2_000_000) -> str:
    if not log_path.is_file():
        return ""
    copy_path = log_path.with_name(log_path.stem + "_llvm_read" + log_path.suffix)
    sources = [log_path]
    try:
        import shutil
        shutil.copy2(log_path, copy_path)
        sources.append(copy_path)
    except OSError:
        pass

    for source in sources:
        for _ in range(5):
            try:
                data = source.read_bytes()
                if len(data) > max_bytes:
                    data = data[-max_bytes:]
                return data.decode("utf-8", errors="replace")
            except PermissionError:
                import time
                time.sleep(0.5)
    return ""
