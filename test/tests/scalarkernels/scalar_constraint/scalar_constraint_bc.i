[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 3
  ny = 3
  elem_type = QUAD4
[]

# NL

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]

  [./alpha]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[ScalarKernels]
  [./alpha_ced]
    type = AlphaCED
    variable = alpha
    value = 10
  [../]
[]

[BCs]
  [./left]
    type = ScalarVarBC
    variable = u
    boundary = '3'
    alpha = alpha
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]
[]

[Preconditioning]
  active = 'pc'

  [./pc]
    type = SMP
    full = true

  solve_type = 'PJFNK'
  [../]

  [./FDP_PJFNK]
    type = FDP
    full = true

  solve_type = 'PJFNK'

    # These options **together** cause a zero pivot in this problem, even without SUPG terms.
    # But using either option alone appears to be OK.
    # petsc_options_iname = '-mat_fd_coloring_err -mat_fd_type'
    # petsc_options_value = '1.e-10               ds'

    petsc_options_iname = '-mat_fd_coloring_err'
    petsc_options_value = '1.e-10'
    # petsc_options_iname = '-mat_fd_type'
    # petsc_options_value = 'ds'
  [../]
[] # End preconditioning block

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  hide = alpha
[]
