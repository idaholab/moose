[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./lambda]
    order=FIRST
    family=SCALAR
  [../]
[]

[ScalarKernels]
  [./alpha]
    type = AlphaCED
    variable = lambda
    value = 0.123
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]

[Debug]
  show_top_residuals = 1
[]
