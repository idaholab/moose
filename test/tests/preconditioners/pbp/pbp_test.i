[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
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

[Preconditioning]
  [./PBP]
    type = PBP
    solve_order = 'u v'
    preconditioner  = 'LU LU'
    off_diag_row    = 'v'
    off_diag_column = 'u'

    petsc_options = ''  # Test petsc options in PBP block
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
    boundary = 3
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 100
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 0
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = Steady

  l_max_its = 10
  nl_max_its = 10

  solve_type = JFNK
[]

[Outputs]
  file_base = out
  exodus = true
[]
