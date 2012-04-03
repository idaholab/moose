[Mesh]
  file = cube-hole.e
[]

[Variables]
  active = 'convected diffused'

  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]

  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'example_diff conv diff'

  [./example_diff]
    type = ExampleCoefDiffusion
    variable = convected
    coef = 0.125
  [../]

  [./conv]
    type = Convection
    variable = convected
    some_variable = diffused
  [../]

  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[BCs]
  active = 'cylinder_convected exterior_convected left_diffused right_diffused'

  [./cylinder_convected]
    type = DirichletBC
    variable = convected
    boundary = '4'
    value = 1
  [../]

  # convected=0 on all vertical sides except the right (x-max)
  [./exterior_convected]
    type = DirichletBC
    variable = convected
    boundary = '5 6 8'
    value = 0
  [../]

  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '5'
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '7'
    value = 10
  [../]

[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'

  l_tol = 1e-3
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-9

  # The adapativity block
  [./Adaptivity]
    steps = 2
    refine_fraction = 0.5 # flags by error fraction
    coarsen_fraction = 0
    max_h_level = 3
    error_estimator = KellyErrorEstimator
    print_changed_info = true
    weight_names = 'convected diffused'
    weight_values = '1.0      0.0'
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]
