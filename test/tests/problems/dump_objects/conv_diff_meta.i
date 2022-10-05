[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[ConvectionDiffusion]
  variables = 'u v'
[]

[Problem]
  type = DumpObjectsProblem
  # diff_u is a meta action added by ConvectionDiffusion
  dump_path = 'diff_u diff_v'
[]

[Executioner]
  type = Transient
[]
