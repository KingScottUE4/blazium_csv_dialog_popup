def can_build(env, platform):
    env.module_add_dependencies("obfuscation", ["mbedtls"], True)
    # Optional: scripts the export plugin rewrites are tokenized like GDScript's own exporter does.
    env.module_add_dependencies("obfuscation", ["gdscript"], True)
    return True


def configure(env):
    pass


def get_doc_classes():
    return [
        "Obfuscation",
        "ObfuscationClaim",
        "ObfuscationScan",
        "ObfuscationKey",
    ]


def get_doc_path():
    return "doc_classes"
