
def _dynamic_libs(ctx):
  for src in ctx.attr.srcs:
    print(src.cc.libs)
  pass

dynamic_libs = rule(
  implementation = _dynamic_libs,
  attrs = {
    "srcs": attr.label_list()
  }
)
