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
    boundary = 'left'
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'right'
    value = 100
  [../]

  [./left_forced]
    type = DirichletBC
    variable = forced
    boundary = 'left'
    value = 0
  [../]

  [./right_forced]
    type = DirichletBC
    variable = forced
    boundary = 'right'
    value = 0
  [../]
[]

[Executioner]
  type = Steady

  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


