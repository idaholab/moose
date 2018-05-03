[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[AddMatAndKernel]
[]

[Problem]
  type = DumpObjectsProblem
  dump_path = AddMatAndKernel
[]

[Executioner]
  type = Steady
[]
