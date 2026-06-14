#!/usr/bin/env python3
"""Parse RPCS3.log for PPU LLVM compile progress."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

MEGA_MODULE_THRESHOLD = 100_000
PER_INSTRUCTION_THRESHOLD = 1_000_000
COMPILE_RE = re.compile(r"LLVM: Compiling module ")
COMPILED_RE = re.compile(r"LLVM: Compiled module ")
LOADED_RE = re.compile(r"LLVM: Loaded module ")
MEGA_RE = re.compile(r"LLVM: (\d+) functions generated \(code_size=0x[0-9a-f]+, num_func=(\d+)")
GHTAM_RE = re.compile(r"GHTAMzWrMaH4uqsLwAwr9h")
JUMP_TABLE_RE = re.compile(r"Jump table not found")
PER_INSTR_TOTAL_RE = re.compile(r"(\d+) instructions will be compiled on per-instruction basis in total")
PER_INSTR_BLOCK_RE = re.compile(r"compiled on per-instruction basis")
POST_LLVM_REGISTER_RE = re.compile(r'_sys_prx_register_module\(name=.*_sysProcessElf')


class HangMode:
    NONE = "none"
    LLVM_MEGA = "llvm_mega_module"
    PER_INSTRUCTION = "per_instruction_fallback"
    POST_LLVM_INIT = "post_llvm_init"


@dataclass
class LlvmProgress:
    compiling: int = 0
    compiled: int = 0
    loaded: bool = False
    mega_module_active: bool = False
    mega_module_seen: bool = False
    max_functions: int = 0
    last_compiling_line: str = ""
    jump_table_warnings: int = 0
    per_instruction_total: int = 0
    per_instruction_blocks: int = 0
    post_llvm_register_seen: bool = False
    hang_mode: str = HangMode.NONE

    @property
    def in_progress(self) -> bool:
        return self.compiling > 0 and not self.loaded

    @property
    def per_instruction_flood(self) -> bool:
        return self.per_instruction_total >= PER_INSTRUCTION_THRESHOLD


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

        if JUMP_TABLE_RE.search(line):
            progress.jump_table_warnings += 1

        per_total = PER_INSTR_TOTAL_RE.search(line)
        if per_total:
            progress.per_instruction_total = max(progress.per_instruction_total, int(per_total.group(1)))

        if PER_INSTR_BLOCK_RE.search(line):
            progress.per_instruction_blocks += 1

        if POST_LLVM_REGISTER_RE.search(line):
            progress.post_llvm_register_seen = True

    if last_compile_line:
        progress.last_compiling_line = last_compile_line
        if GHTAM_RE.search(last_compile_line) and progress.compiling > 0:
            progress.mega_module_active = True

    progress.hang_mode = classify_hang_mode(progress)
    return progress


def classify_hang_mode(progress: LlvmProgress) -> str:
    if progress.compiling > 0 or progress.compiled > 0:
        return HangMode.LLVM_MEGA
    if progress.loaded and progress.post_llvm_register_seen:
        return HangMode.POST_LLVM_INIT
    if progress.per_instruction_flood:
        return HangMode.PER_INSTRUCTION
    if progress.per_instruction_total > 0 and progress.jump_table_warnings > 50 and not progress.loaded:
        return HangMode.PER_INSTRUCTION
    return HangMode.NONE


def format_llvm_status(progress: LlvmProgress) -> list[str]:
    lines = [
        f"  LLVM modules compiled: {progress.compiled}",
        f"  LLVM modules in progress: {progress.compiling}",
        f"  LLVM loaded into main_thread: {'yes' if progress.loaded else 'no'}",
        f"  Jump table warnings: {progress.jump_table_warnings}",
        f"  Per-instruction blocks: {progress.per_instruction_blocks}",
    ]
    if progress.per_instruction_total > 0:
        lines.append(f"  Per-instruction total: {progress.per_instruction_total:,}")
    if progress.max_functions > 0:
        lines.append(f"  LLVM max functions in one module: {progress.max_functions:,}")
    if progress.mega_module_active:
        lines.append("  LLVM mega-module active: yes (low CPU for 5-10 min is normal — do not close RPCS3)")
    elif progress.mega_module_seen:
        lines.append("  LLVM mega-module seen: yes (already compiled or loading)")
    if progress.hang_mode == HangMode.PER_INSTRUCTION:
        lines.append("  Hang mode: per-instruction fallback (rebuild with rpcs3_compat=no)")
    elif progress.hang_mode == HangMode.LLVM_MEGA:
        lines.append("  Hang mode: LLVM mega-module compile (wait 5-10 min)")
    elif progress.hang_mode == HangMode.POST_LLVM_INIT:
        lines.append("  Hang mode: post-LLVM CRT init (check USRDIR/boot.log for early_init)")
    if progress.last_compiling_line:
        short = progress.last_compiling_line
        if len(short) > 120:
            short = short[:117] + "..."
        lines.append(f"  LLVM last compile line: {short}")
    return lines


def format_hang_advice(progress: LlvmProgress, llvm_recompiler: bool = True) -> list[str]:
    advice: list[str] = []
    if progress.hang_mode == HangMode.PER_INSTRUCTION:
        advice.append(
            "FAIL: Per-instruction LLVM fallback detected — RPCS3 appears hung but is analyzing millions of blocks."
        )
        advice.append("      Rebuild template: bash platform/ps3/scripts/build_ps3_llvm_release.sh (rpcs3_compat=no).")
    elif progress.hang_mode == HangMode.LLVM_MEGA:
        if progress.mega_module_active:
            advice.append("INFO: LLVM mega-module compiling — wait 5-10 min (low CPU is normal). Do not close RPCS3.")
        else:
            advice.append("INFO: PPU LLVM cache still building — first boot can take 30-40 min total.")
    elif progress.hang_mode == HangMode.POST_LLVM_INIT:
        advice.append("WARN: Post-LLVM init stall — check TTY and USRDIR/boot.log for early_init / modules_loaded.")
    if progress.per_instruction_flood and llvm_recompiler and progress.compiled == 0 and progress.compiling == 0:
        advice.append(
            f"FAIL: {progress.per_instruction_total:,} instructions queued for per-instruction compile with LLVM config."
        )
    elif progress.per_instruction_total > 1_000_000 and (progress.compiling > 0 or progress.compiled > 0):
        advice.append(
            f"INFO: Per-instruction analysis reported {progress.per_instruction_total:,} blocks; LLVM module compile is in progress."
        )
    return advice


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
