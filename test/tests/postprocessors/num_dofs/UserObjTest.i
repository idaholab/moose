[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
[]



[Postprocessors]
  [./num_dofs_nl]
    type = NumDOFs
    system = NL
  [../]
[]

[Outputs]
  csv = true
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]
