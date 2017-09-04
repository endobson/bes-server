
def _grpc_cc_library_impl(ctx):
  include_dirs = dict()
  for file in ctx.attr.srcs[0].proto.transitive_sources:
    if (file.short_path.startswith("../")):
      index = file.short_path.index("/", 3)
      if (file.root.path == ""):
         include_dirs["external/" + file.short_path[3:index]] = True
      else:
         include_dirs[file.root.path + "/external/" + file.short_path[3:index]] = True
    else:
      include_dirs[file.root.path] = True
 
  srcs = []
  for src in ctx.attr.srcs[0].proto.direct_sources:
    for dir in include_dirs.keys():
      if src.path.startswith(dir):
        srcs += [src.path[len(dir) + 1:]]
        break

  outputs = []
  output_roots = {}
  for proto_src in ctx.attr.proto_srcs:
    for suffix in ["_header", "_cc"]:
      output = getattr(ctx.outputs, proto_src.label.name + suffix)
      output_roots[output.root.path] = True
      outputs.append(output)
  if (len(output_roots.keys()) != 1):
    fail("Output roots must all be the same.")

  output_root = output_roots.keys()[0]

  ctx.actions.run(
    outputs = outputs,
    inputs = ctx.attr.srcs[0].proto.transitive_sources + [ctx.file.grpc_plugin],
    executable = ctx.file.protoc,
    arguments = [
      "--grpc_out=" + output_root + "/external/googleapis/",
      "--plugin=protoc-gen-grpc=" + ctx.file.grpc_plugin.path,
    ] + 
    ["-I" + dir for dir in include_dirs.keys()] +
    srcs
  )

  return [] 

def _grpc_cc_library_outputs(proto_srcs):
  outputs = {}
  for proto_src in proto_srcs:
    prefix = proto_src.name[:-len(".proto")]
    outputs[proto_src.name + "_header"] = prefix + ".grpc.pb.h"
    outputs[proto_src.name + "_cc"] = prefix + ".grpc.pb.cc"
  return outputs 

grpc_cc_library = rule(
  implementation = _grpc_cc_library_impl,
  attrs = {
    "srcs" : attr.label_list(),
    "proto_srcs" : attr.label_list(allow_files=[".proto"]),
    "protoc" : attr.label(default="@com_google_protobuf//:protoc", single_file=True),
    "grpc_plugin" : attr.label(default="@grpc//:grpc_cpp_plugin", single_file=True)
  },
  output_to_genfiles = True,
  outputs = _grpc_cc_library_outputs
)

