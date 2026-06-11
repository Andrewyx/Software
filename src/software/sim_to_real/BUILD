load("@rules_python//python:defs.bzl", "py_binary")
load("@rules_python//python:pip.bzl", "compile_pip_requirements")
load("@sim_to_real_deps//:requirements.bzl", "requirement")

package(default_visibility = ["//software/sim_to_real:__subpackages__"])

py_binary(
    name = "extract_kinematics_events",
    srcs = ["extract_kinematics_events.py"],
    deps = [
        "//proto:import_all_protos",
        "//software/thunderscope/replay:proto_player",
        requirement("pandas"),
    ],
)

py_binary(
    name = "extract_game_events",
    srcs = ["extract_game_events.py"],
    deps = [
        "//proto:import_all_protos",
        "//software/thunderscope/replay:proto_player",
        requirement("pandas"),
    ],
)

py_binary(
    name = "optuna_physics_tuner",
    srcs = ["optuna_physics_tuner.py"],
    deps = [
        requirement("optuna"),
    ],
)

py_binary(
    name = "optuna_gameplay_tuner",
    srcs = ["optuna_gameplay_tuner.py"],
    deps = [
        requirement("optuna"),
    ],
)

py_binary(
    name = "auto_tune",
    srcs = ["auto_tune.py"],
    deps = [],
)

compile_pip_requirements(
    name = "requirements",
    src = "requirements.in",
    requirements_txt = "requirements_lock.txt",
)
