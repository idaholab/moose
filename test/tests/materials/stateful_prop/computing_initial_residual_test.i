[Mesh]
  dim = 3
  file = cube.e
[]

[Variables]
  active = 'u prop1'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./prop1]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'heat ie prop1_output'

  [./heat]
    type = MatDiffusion
    variable = u
    prop_name = thermal_conductivity
  [../]

  [./ie]
    type = TimeDerivative
    variable = u
  [../]

  [./prop1_output]
    type = RealPropertyOutput
    variable = prop1
    prop_name = thermal_conductivity
  [../]
[]

[BCs]
  active = 'bottom top'

  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0.0
  [../]

  [./top]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1.0
  [../]
[]

[Materials]
  active = 'stateful'

  [./stateful]
    type = ComputingInitialTest
    block = 1
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator'

  l_max_its = 10

  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Output]
  file_base = computing_initial_residual_test_out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]

