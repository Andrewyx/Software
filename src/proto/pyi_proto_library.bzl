load("@rules_proto//proto:defs.bzl", "ProtoInfo")

def _pyi_proto_aspect_impl(target, ctx):
    proto_info = target[ProtoInfo]
    
    # Generate the .pyi output files based on the input .proto files
    generated_sources = []
    for proto in proto_info.direct_sources:
        # Construct the output filename (replace .proto with _pb2.pyi)
        out_name = proto.basename[:-len(".proto")] + "_pb2.pyi"
        out_file = ctx.actions.declare_file(out_name, sibling=proto)
        generated_sources.append(out_file)

    if not generated_sources:
        return []

    # Get the transitive descriptors (all .proto files this depends on)
    transitive_descriptor_sets = proto_info.transitive_descriptor_sets

    args = ctx.actions.args()
    
    # Tell protoc to generate python stubs via the mypy plugin
    args.add("--plugin=protoc-gen-mypy=" + ctx.executable._mypy_plugin.path)
    
    # We output to the bin directory at the root of the workspace to preserve package paths
    out_dir = ctx.bin_dir.path + "/" + ctx.label.workspace_root
    args.add("--mypy_out=" + out_dir)

    # Add all include paths from the transitive descriptors
    args.add_joined(
        "--descriptor_set_in",
        transitive_descriptor_sets,
        join_with = ctx.configuration.host_path_separator,
    )
    
    # Add the source files to compile
    for src in proto_info.direct_sources:
        args.add(src.path)

    ctx.actions.run(
        inputs = depset(
            direct = proto_info.direct_sources,
            transitive = [transitive_descriptor_sets]
        ),
        outputs = generated_sources,
        executable = ctx.executable._protoc,
        tools = [ctx.executable._mypy_plugin],
        arguments = [args],
        progress_message = "Generating Python stubs for %s" % ctx.label,
    )

    return [DefaultInfo(files = depset(direct = generated_sources))]

_pyi_proto_aspect = aspect(
    implementation = _pyi_proto_aspect_impl,
    attr_aspects = ["deps"],
    required_providers = [ProtoInfo],
    attrs = {
        "_protoc": attr.label(
            default = Label("@protobuf//:protoc"),
            executable = True,
            cfg = "exec",
        ),
        "_mypy_plugin": attr.label(
            default = Label("//proto/python_stubs:protoc_gen_mypy"),
            executable = True,
            cfg = "exec",
        ),
    },
)

def _pyi_proto_library_rule_impl(ctx):
    all_files = []
    for dep in ctx.attr.deps:
        all_files.append(dep[DefaultInfo].files)
        
    return [DefaultInfo(
        files = depset(transitive = all_files)
    )]

pyi_proto_library = rule(
    implementation = _pyi_proto_library_rule_impl,
    attrs = {
        "deps": attr.label_list(
            providers = [ProtoInfo],
            aspects = [_pyi_proto_aspect],
        ),
    },
)