[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[Problem]
  solve = false
[]

[MultiApps]
  [sub]
    type = OverrideCliArgs
    input_files = sub.i
    xmax = 12
  []
[]

[Executioner]
  type = Steady
[]
