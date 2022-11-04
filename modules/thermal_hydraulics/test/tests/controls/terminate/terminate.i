[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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

[Postprocessors]
  [dt_pp]
    type = TimestepSize
  []
[]

[Components]
[]

[ControlLogic]
  [threshold]
    type = UnitTripControl
    condition = 'dt_pp > 3'
    symbol_names = 'dt_pp'
    symbol_values = 'dt_pp'
  []

  [terminate]
    type = TerminateControl
    input = threshold:state
    termination_message = 'Threshold exceeded'
  []
[]

[Functions]
  [dt_fn]
    type = ParsedFunction
    expression = '1 + t'
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = dt_fn
  []
  num_steps = 10
  abort_on_solve_fail = true
[]
