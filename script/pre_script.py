Import("env")

exclude = [
  "system_stm32f0xx.c",
  "system_stm32f1xx.c"
]

def replace_system(node):
  if node.name in exclude:
    print("dropping ", node)
    return None

  return node

env.AddBuildMiddleware(
  replace_system,
  "*/framework-cmsis*/*"
)

env.Append(
  CFLAGS=["--model-small"],
  LINKFLAGS=["--model-small", "--opt-code-speed", "--xram-loc", "0xf000", "--code-loc", "0x0"]
)