[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Variables]
  [./x]
    family = SCALAR
    order = FIRST
    initial_condition = 0
  [../]
[]

[ScalarKernels]
  [./dt]
    type = ODETimeDerivative
    variable = x
  [../]
  [./ode1]
    type = ParsedODEKernel
    expression = '-mytime'
    postprocessors = mytime
    variable = x
  [../]
[]

[Postprocessors]
  [./computed_x]
    type = ScalarVariable
    variable = x
    execute_on = 'initial timestep_end'
  [../]

  [./mytime]
    type = FunctionValuePostprocessor
    function = t
    execute_on = 'initial timestep_begin'
  [../]

  [./exact_x]
    type = FunctionValuePostprocessor
    function = '0.5*t^2'
    execute_on = 'initial timestep_end'
  [../]

  [./l2err_x]
    type = ScalarL2Error
    variable = x
    function = '0.5*t^2'
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  dt = 0.1
  num_steps = 10

  solve_type = 'NEWTON'
[]

[Outputs]
  file_base = ode_pp_test_out
  hide = 'x mytime'
  csv = true
[]
