[Problem]
  type = FEProblem
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
[]

[AuxVariables]
  [u]
  []
  [w]
  []
  [v_x]
  []
  [v_y]
  []
[]

[AuxKernels]
  [u]
    type = FunctionAux
    variable = u
    function = u
  []
  [w]
    type = FunctionAux
    variable = w
    function = w
  []
  [v_x]
    type = FunctionAux
    variable = v_x
    function = v_x
  []
  [v_y]
    type = FunctionAux
    variable = v_y
    function = v_y
  []
[]

[Functions]
  [u] # reaches a maximum value at (0.5, 0.6)
    type = ParsedFunction
    expression = 'sin(pi*x)*sin(pi*y/1.2)'
  []
  [w] # reaches a minium expression at (0.7, 0.8)
    type = ParsedFunction
    expression = '-sin(pi*x/1.4)*sin(pi*y/1.6)'
  []

  [v_x]
    type = ParsedFunction
    expression = 'x'
  []
  [v_y]
    type = ParsedFunction
    expression = 'y'
  []
[]

[Postprocessors]
  # because we set v_x and v_y equal to the x and y coordinates, these two postprocessors
  # should just return the point at which u reaches a maximum value
  [max_v_from_proxy_x]
    type = NodalExtremeValue
    variable = v_x
    proxy_variable = u
    value_type = max
  []
  [max_v_from_proxy_y]
    type = NodalExtremeValue
    variable = v_y
    proxy_variable = u
    value_type = max
  []

  # because we set v_x and v_y equal to the x and y coordinates, these two postprocessors
  # should just return the point at which w reaches a minimum value
  [min_v_from_proxy_x]
    type = NodalExtremeValue
    variable = v_x
    proxy_variable = w
    value_type = min
  []
  [min_v_from_proxy_y]
    type = NodalExtremeValue
    variable = v_y
    proxy_variable = w
    value_type = min
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  csv = true
[]
