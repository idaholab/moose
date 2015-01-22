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

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    output_on = 'initial failed linear nonlinear timestep_end'
  [../]
  [./debug]
    type = TopResidualDebugOutput
    num_residuals = 1
  [../]
[]
