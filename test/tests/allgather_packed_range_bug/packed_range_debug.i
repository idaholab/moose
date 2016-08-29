[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[UserObjects]
  [./pack]
    type = PackedRangeOverflow
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
