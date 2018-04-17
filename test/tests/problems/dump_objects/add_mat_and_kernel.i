[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[AddMatAndKernel]
  family = HERMITE
[]

[Problem]
  type = DumpObjectsProblem
  dump_path = AddMatAndKernel
[]

[Executioner]
  type = Steady
[]
