# This test takes a value of (a) function, (b) postprocessor, (c) scalar variable,
# (d) real-valued control value and (f) bool-valued control value and evaluates it via
# ParsedFunctionControl object

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [pps_fn]
    type = ConstantFunction
    value = 4
  []

  [fn]
    type = ConstantFunction
    value = 5
  []
[]

[AuxVariables]
  [sv]
    family = SCALAR
    order = FIRST
    initial_condition = 0
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[AuxScalarKernels]
  [sv_ak]
    type = ConstantScalarAux
    variable = sv
    value = 3
    execute_on = 'timestep_begin'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Components]
[]

[Postprocessors]
  [pps]
    type = FunctionValuePostprocessor
    function = pps_fn
    execute_on = 'timestep_begin'
  []

  [result]
    type = RealControlDataValuePostprocessor
    control_data_name = eval_ctrl:value
    execute_on = 'timestep_end'
  []
[]

[ControlLogic]
  [ctrl]
    type = GetFunctionValueControl
    function = 2
  []

  [trip]
    type = UnitTripControl
    condition = 't > 0'
  []

  [eval_ctrl]
    type = ParsedFunctionControl
    function = 'a + b + c + d + f'
    symbol_names = 'a b c d f'
    symbol_values = 'fn pps sv ctrl:value trip:state'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
  abort_on_solve_fail = true
[]

[Outputs]
  csv = true
  show = 'result'
[]
