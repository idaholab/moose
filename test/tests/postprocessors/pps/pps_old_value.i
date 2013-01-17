[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  [../]
[]

[Functions]
  [./force_fn]
    type = ParsedFunction
    value = '1'
  [../]

  [./exact_fn]
    type = ParsedFunction
    value = 't'
  [../]
[]

[Kernels]
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./ffn_u]
    type = UserForcingFunction
    variable = u
    function = force_fn
  [../]
[]

[BCs]
  [./all_u]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./a]
    type = ElementIntegralVariablePostprocessor
    variable = u
  [../]

  [./total_a]
    type = TotalVariableValue
    value = a
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator'

  dt = 1
  start_time = 1
  end_time = 3
[]

[Output]
  output_initial = false
  postprocessor_csv = false
  interval = 1
  exodus = true
  print_linear_residuals = false
  perf_log = true
[]


