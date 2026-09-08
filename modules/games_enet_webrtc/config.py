def can_build(env, platform):
    env.module_add_dependencies("games_enet_webrtc", ["enet", "webrtc", "websocket"])
    return True


def configure(env):
    pass


def is_enabled():
    return True


def get_doc_classes():
    return [
        "WebRTCEnetSession",
        "SignalClient",
    ]


def get_doc_path():
    return "doc_classes"
