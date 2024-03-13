[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 5
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
