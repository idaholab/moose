[Mesh]
  file = square.e
#  init_unif_refine = 6
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

[Problem]
  type = FEProblem
  error_on_jacobian_nonzero_reallocation = true
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

[Preconditioning]
  [./pbp]
    type = PBP
    solve_order = 'u v'
    preconditioner  = 'LU LU'
    off_diag_row    = 'v'
    off_diag_column = 'u'

    petsc_options = ''  # Test petsc options in PBP block
  [../]
[]

# [Preconditioning]
#   [./smp]
#     type = SMP
#     full = true
#   [../]
# []

[Executioner]
  type = Steady
  solve_type = JFNK

  # solve_type = 'NEWTON'

  # petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  # petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  # petsc_options_value = 'lu NONZERO   1e-15'

  # petsc_options_iname = '-pc_type -ksp_view_mat'
  # petsc_options_value = 'svd ascii:matrix.m:ascii_matlab'


  l_max_its = 10
  nl_max_its = 10
[]

[Outputs]
  [./exodus]
    file_base = condense_test
    type = Exodus
  [../]
  [dof_map]
    file_base = condense_test
    type = DOFMap
    execute_on = 'initial'
  []
[]
