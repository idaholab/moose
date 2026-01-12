[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 20
[]

[AuxVariables]
  [f]
  []
[]

[AuxKernels]
  [function_aux]
    type = FunctionAux
    variable = f
    function = fn
    execute_on = initial
  [../]
[]

[Functions]
  [sin_fn]
    type = ParsedFunction
    expression = sin(x)
  []
  [cos_fn]
    type = ParsedFunction
    expression = cos(x)
  []
  [fn]
    type = ParsedFunction
    # Inputs we will use in the parsed expression
    symbol_values = 'sin_fn cos_fn'
    # Short names for each input
    symbol_names = 's c'
    # Expression to parse
    expression = 's/c'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  show = f
  exodus = true
[]
