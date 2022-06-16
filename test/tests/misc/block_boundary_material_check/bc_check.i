[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[BCs]
  [./bc_left]
    type = MatTestNeumannBC
    variable = u
    boundary = left
    mat_prop = 'prop'
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]
