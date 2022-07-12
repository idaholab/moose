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
  [./debug] # This is only test, use [Debug] block to enable this
    type = TopResidualDebugOutput
    num_residuals = 1
  [../]
[]
