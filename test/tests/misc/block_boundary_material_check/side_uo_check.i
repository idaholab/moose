[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[UserObjects]
  [./side_uo]
    type = MatSideUserObject
    mat_prop = 'foo'
    boundary = 1
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
