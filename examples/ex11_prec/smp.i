[Mesh]
  file = square.e
[]

[Variables]
  active = 'diffused forced'

  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]

  [./forced]
    order = FIRST
    family = LAGRANGE
  [../]
[]

# The Preconditioning block
[Preconditioning]
  active = 'SMP_jfnk'

  [./SMP_jfnk]
    type = SMP

    off_diag_row    = 'forced'
    off_diag_column = 'diffused'


  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  [../]

  [./SMP_jfnk_full]
    type = SMP

    full = true


  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  [../]

  [./SMP_n]
    type = SMP

    off_diag_row    = 'forced'
    off_diag_column = 'diffused'


    solve_type = 'NEWTON'

    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  [../]
[]

[Kernels]
  active = 'diff_diffused conv_forced diff_forced'

  [./diff_diffused]
    type = Diffusion
    variable = diffused
  [../]

  [./conv_forced]
    type = CoupledForce
    variable = forced
    v = diffused
  [../]

  [./diff_forced]
    type = Diffusion
    variable = forced
  [../]
[]

[BCs]
  active = 'left_diffused right_diffused left_forced'

  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 1
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 2
    value = 100
  [../]

  [./left_forced]
    type = DirichletBC
    variable = forced
    boundary = 1
    value = 0
  [../]

  [./right_forced]
    type = DirichletBC
    variable = forced
    boundary = 2
    value = 0
  [../]
[]

[Executioner]
  type = Steady


  print_linear_residuals = true

[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


