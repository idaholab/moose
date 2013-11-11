[Mesh]
  file = 2squares.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./exception]
    type = ExceptionKernel
    variable = u
    when = jacobian
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = ExceptionSteady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  linear_residuals = true
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


