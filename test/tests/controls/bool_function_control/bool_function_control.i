[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [solve_fn]
    type = ParsedFunction
    expression = 'if(t<0.3, 1, 0)'
  []
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[Kernels]
  [td]
    type = TimeDerivative
    variable = u
  []
  [bf]
    type = BodyForce
    variable = u
    function = 1
  []
[]

[Controls]
  [solve_ctrl]
    type = BoolFunctionControl
    function = solve_fn
    parameter = '*/*/solve'
    execute_on = timestep_begin
  []
[]

[Postprocessors]
  [./u_val]
    type = ElementAverageValue
    variable = u
    execute_on = 'initial timestep_begin'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
[]

[Outputs]
  csv = true
[]
