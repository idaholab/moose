[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[Functions]
  [gfun]
    type = ParsedFunction
    expression = '2*x'
  []
  [cfun]
    type = ParsedFunction
    expression = '0.5'
  []
[]

[Postprocessors]
  [pp]
    type = FunctionValuePostprocessor
    function = cfun
    execute_on = 'INITIAL'
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [move]
    type = MoveNodesByParsedExpression
    block = 0
    displacement_x = '0.1*gfun'
    displacement_z = '0.3*pp'
    functions = 'gfun'
    postprocessors = 'pp'
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  exodus = true
[]
