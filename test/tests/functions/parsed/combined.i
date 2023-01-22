[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 20
[]

[AuxVariables]
  [./f]
  [../]

  [./sv]
    family = SCALAR
    order = FIRST
    initial_condition = 100
  [../]
[]

[AuxKernels]
  [./function_aux]
    type = FunctionAux
    variable = f
    function = fn
  [../]
[]

[Functions]
  [./pp_fn]
    type = ParsedFunction
    expression = '2*(t+1)'
  [../]
  [./cos_fn]
    type = ParsedFunction
    expression = 'cos(pi*x)'
  [../]
  [./fn]
    type = ParsedFunction
    expression = 'scalar_expression * func / pp'
    symbol_names = 'scalar_expression func   pp'
    symbol_values = 'sv           cos_fn pp'
  [../]
[]

[Postprocessors]
  [./pp]
    type = FunctionValuePostprocessor
    function = pp_fn
    execute_on = initial
  [../]
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
  execute_on = final
[]
