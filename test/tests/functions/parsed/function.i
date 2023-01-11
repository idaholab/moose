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
    expression = 's/c'
    symbol_names = 's c'
    symbol_values = 'sin_fn cos_fn'
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
