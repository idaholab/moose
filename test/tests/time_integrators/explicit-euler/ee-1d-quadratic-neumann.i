[GlobalParams]
  implicit = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 10
  elem_type = EDGE3
[]

[Functions]
  [./ic]
    type = ParsedFunction
    value = 0
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = x*x-2*t+t*x*x
  [../]

  [./exact_fn]
    type = ParsedFunction
    value = t*x*x
  [../]

  [./left_bc_fn]
    type = ParsedFunction
    value = -t*2*x
  [../]
  [./right_bc_fn]
    type = ParsedFunction
    value = t*2*x
  [../]
[]

[Variables]
  [./u]
    order = SECOND
    family = LAGRANGE

    [./InitialCondition]
      type = FunctionIC
      function = ic
    [../]
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
    implicit = true
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./abs]
    type = Reaction
    variable = u
  [../]

  [./ffn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./left]
    type = FunctionNeumannBC
    variable = u
    boundary = '0'
    function = left_bc_fn
  [../]

  [./right]
    type = FunctionNeumannBC
    variable = u
    boundary = '1'
    function = right_bc_fn
  [../]
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'explicit-euler'
  solve_type = 'LINEAR'

  l_tol = 1e-12
  start_time = 0.0
  num_steps = 10
  dt = 0.001
[]

[Output]
  output_initial = true
  interval = 1
  exodus = true
  max_pps_rows_screen = 10
[]

