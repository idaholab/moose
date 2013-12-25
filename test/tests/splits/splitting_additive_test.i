[Mesh]
  file = square.e
[]

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff_u conv_v diff_v'

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./conv_v]
    type = CoupledForce
    variable = v
    v = u
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  active = 'left_u right_u left_v'

  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 100
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 0
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  splitting = 'uv'
  petsc_options       = '-snes_view -snes_monitor -snes_converged_reason -ksp_converged_reason'
[]
[Splits]
  [./uv]
    splitting = 'u v'
    splitting_type  = additive
    petsc_options = '-dm_view'
  [../]
  [./u]
    vars = 'u'
    petsc_options       = '-dm_view'
    petsc_options_iname = '-pc_type'
    petsc_options_value = '     asm'
  [../]
  [./v]
    vars = 'v'
    petsc_options       = '-dm_view'
    petsc_options_iname = '-pc_type'
    petsc_options_value = '     asm'
  [../]
[]

[Output]
  linear_residuals = true
  file_base = splitting_additive_out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


