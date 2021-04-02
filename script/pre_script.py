Import("env")

exclude = [
  "system_stm32f0xx.c"
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