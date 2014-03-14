[Mesh]
  file = rectangle.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff body_force'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./body_force]
    type = BodyForce
    variable = u
    block = 1
    value = 10
  [../]
[]

[BCs]
  active = 'right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
#  solve_type = 'PJFNK'
#  preconditioner = 'ILU'

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  file_base = out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
