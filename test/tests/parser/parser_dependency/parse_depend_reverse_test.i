[Outputs]
  file_base = 2d_diffusion_reverse_out
  exodus = true
  output_on = 'initial timestep_end'
  [./console]
    type = Console
    perf_log = true
    output_on = 'timestep_end failed nonlinear'
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Mesh]
  dim = 2
  file = square.e
[]
