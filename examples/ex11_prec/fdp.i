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
  active = 'FDP_jfnk'

  [./FDP_jfnk]
    type = FDP

    off_diag_row    = 'forced'
    off_diag_column = 'diffused'

    petsc_options = '-snes_mf_operator'
    petsc_options_iname = '-pc_type -mat_fd_coloring_err -mat_fd_type'
    petsc_options_value = 'lu       1e-6                 ds'
  [../]

  [./FDP_n]
    type = FDP

    off_diag_row    = 'forced'
    off_diag_column = 'diffused'

    petsc_options = '-snes'
    petsc_options_iname = '-pc_type -mat_fd_coloring_err -mat_fd_type'
    petsc_options_value = 'lu       1e-6                 ds'
  [../]

  [./FDP_n_full]
    type = FDP

    full = true

    petsc_options = '-snes'
    petsc_options_iname = '-pc_type -mat_fd_coloring_err -mat_fd_type'
    petsc_options_value = 'lu       1e-6                 ds'
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

  petsc_options = '-ksp_monitor'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
   
    
