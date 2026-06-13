import os
import sys
from typing import TYPE_CHECKING

from methods import print_error, print_info, print_warning
from platform_methods import validate_arch

if TYPE_CHECKING:
    from SCons.Script.SConscript import SConsEnvironment


def get_name():
    return "PS3"


def _get_ps3dev():
    return os.environ.get("PS3DEV", "/usr/local/ps3dev")


def _get_psl1ght():
    return os.environ.get("PSL1GHT", _get_ps3dev())


def can_build():
    ps3dev = _get_ps3dev()
    psl1ght = _get_psl1ght()

    if not os.path.isdir(ps3dev):
        print_error("PS3DEV not found at '%s'. PS3 platform disabled." % ps3dev)
        return False

    if not os.path.isdir(psl1ght):
        print_error("PSL1GHT not found at '%s'. PS3 platform disabled." % psl1ght)
        return False

    ppu_gcc = os.path.join(ps3dev, "ppu", "bin", "ppu-gcc")
    if not os.path.isfile(ppu_gcc) and os.system("ppu-gcc --version > /dev/null 2>&1") != 0:
        print_error("ppu-gcc not found. PS3 platform disabled.")
        return False

    if not os.path.isfile(os.path.join(psl1ght, "ppu", "lib", "lv2.ld")):
        print_error("PSL1GHT lv2.ld not found. PS3 platform disabled.")
        return False

    if not os.path.isdir(os.path.join(psl1ght, "ppu", "include")):
        print_error("PSL1GHT headers not found. PS3 platform disabled.")
        return False

    return True


def get_opts():
    from SCons.Variables import BoolVariable, EnumVariable

    return [
        BoolVariable("norsx", "Use NoRSX software framebuffer", True),
        BoolVariable("rsx", "Enable RSX/GCM hardware renderer (experimental)", False),
        EnumVariable("debug_symbols", "Add debugging symbols to release builds", "yes", ("yes", "no", "full")),
        BoolVariable("separate_debug_symbols", "Create a separate file containing debugging symbols", False),
    ]


def get_doc_classes():
    return [
        "EditorExportPlatformPS3",
        "PlayStation3",
    ]


def get_doc_path():
    return "doc_classes"


def get_flags():
    return {
        "arch": "ppc64",
        "tools": False,
        "vulkan": False,
        "opengl3": False,
        "use_sowrap": False,
        "module_raycast_enabled": False,
        "module_gdextension_enabled": False,
        "module_websocket_enabled": False,
        "builtin_zlib": False,
        "builtin_freetype": False,
        "builtin_libpng": False,
        "builtin_libwebp": False,
        "builtin_mbedtls": False,
        "builtin_miniupnpc": False,
        "builtin_enet": True,
        "builtin_bullet": True,
        "builtin_pcre2": True,
        "optimize": "speed",
    }


def configure(env: "SConsEnvironment"):
    ps3dev = _get_ps3dev()
    psl1ght = _get_psl1ght()
    portlibs = os.path.join(ps3dev, "portlibs", "ppu")

    validate_arch(env["arch"], get_name(), ["ppc64"])

    env["ENV"]["PS3DEV"] = ps3dev
    env["ENV"]["PSL1GHT"] = psl1ght
    ppu_bin = os.path.join(ps3dev, "ppu", "bin")
    spu_bin = os.path.join(ps3dev, "spu", "bin")
    host_bin = os.path.join(ps3dev, "bin")
    updated_path = ":".join(
        [
            ppu_bin,
            spu_bin,
            host_bin,
            os.environ.get("PATH", ""),
        ]
    )
    env["ENV"]["PATH"] = updated_path
    os.environ["PATH"] = updated_path

    env["CC"] = "ppu-gcc"
    env["CXX"] = "ppu-g++"
    env["AR"] = "ppu-ar"
    env["RANLIB"] = "ppu-ranlib"
    env["LINK"] = "ppu-g++"

    env.Append(CPPPATH=["#platform/ps3"])
    env.Append(
        CPPPATH=[
            os.path.join(psl1ght, "ppu", "include"),
            os.path.join(portlibs, "include"),
        ]
    )
    env.Append(
        LIBPATH=[
            os.path.join(psl1ght, "ppu", "lib"),
            os.path.join(portlibs, "lib"),
        ]
    )

    env.Append(
        CPPDEFINES=[
            "PS3_ENABLED",
            "__PS3__",
            "__lv2ppu__",
            "__PPU__",
            "__powerpc64__",
            "UNIX_ENABLED",
            "PTHREAD_ENABLED",
        ]
    )

    env.Append(CCFLAGS=["-mcpu=cell", "-ffunction-sections", "-fdata-sections"])
    env.Append(LINKFLAGS=["-m elf64ppc", "-Wl,--gc-sections"])

    env.Append(LIBS=["sysbase", "c", "rt", "lv2", "pad", "sysutil", "sysmodule"])

    norsx_lib = os.path.join(portlibs, "lib", "libnorsx.a")
    if env["norsx"] and os.path.isfile(norsx_lib):
        env.Append(CPPDEFINES=["NORSX_ENABLED"])
        env.Append(LIBS=["norsx"])
        print_info("PS3: NoRSX enabled from portlibs.")
    elif env["norsx"]:
        print_warning("PS3: NoRSX library not found; using software framebuffer stub.")

    ps3sound_lib = os.path.join(portlibs, "lib", "libps3sound.a")
    if os.path.isfile(ps3sound_lib):
        env.Append(CPPDEFINES=["PS3SOUND_ENABLED"])
        env.Append(LIBS=["ps3sound"])
        print_info("PS3: ps3soundlib enabled from portlibs.")

    if env["rsx"]:
        env.Append(CPPDEFINES=["RSX_ENABLED"])
        env.Append(LIBS=["gcm_sys", "rsx"])
        print_info("PS3: RSX/GCM hardware renderer enabled (experimental).")

    if os.path.isdir(os.path.join(portlibs, "lib", "pkgconfig")):
        env["ENV"]["PKG_CONFIG_PATH"] = os.path.join(portlibs, "lib", "pkgconfig")

    env.extra_suffix = ".ps3" + env.extra_suffix

    if env["debug_symbols"] == "no":
        env.Append(CCFLAGS=["-g0"])
    elif env["debug_symbols"] == "full":
        env.Append(CCFLAGS=["-g3"])
    else:
        env.Append(CCFLAGS=["-g1"])

    print_info("PS3 platform configured (PS3DEV=%s, PSL1GHT=%s)" % (ps3dev, psl1ght))
