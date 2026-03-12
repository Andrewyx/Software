load("@rules_python//python:pip.bzl", "compile_pip_requirements")

compile_pip_requirements(
    name = "requirements",
    src = "requirements.in",
    requirements_txt = "requirements_lock.txt",
)

load("@proto_stubs_deps//:requirements.bzl", "requirement")

py_binary(
    name = "protoc_gen_mypy",
    srcs = ["protoc_gen_mypy_wrapper.py"],
    main = "protoc_gen_mypy_wrapper.py",
    deps = [requirement("mypy-protobuf")],
)

package(default_visibility = ["//visibility:public"])
