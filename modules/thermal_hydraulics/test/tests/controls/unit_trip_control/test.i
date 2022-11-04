[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [fn]
    type = ParsedFunction
    expression = 'sin(pi*t)'
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
  [a]
    type = FunctionValuePostprocessor
    function = fn
    execute_on = 'timestep_begin'
  []

  [trip_state]
    type = BoolControlDataValuePostprocessor
    control_data_name = trip_ctrl:state
    execute_on = 'timestep_end'
  []
[]

[ControlLogic]
  [trip_ctrl]
    type = UnitTripControl
    condition = 'a > 0.6'
    symbol_names = 'a'
    symbol_values = 'a'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 10
  abort_on_solve_fail = true
[]

[Outputs]
  csv = true
[]
