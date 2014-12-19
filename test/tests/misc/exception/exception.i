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

[Outputs]
  exodus = true
  output_on = 'initial timestep_end'
  [./console]
    type = Console
    perf_log = true
    output_on = 'timestep_end failed nonlinear linear'
  [../]
[]
