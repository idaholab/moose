[Mesh]
  file = square.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./nan]
    type = NanKernel
    variable = u
    timestep_to_nan = 2
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
  type = Transient

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  dt = 1
  num_steps = 5
[]

[Output]
  linear_residuals = true
  output_initial = true
  exodus = true
  perf_log = true
[]
