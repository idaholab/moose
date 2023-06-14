[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Variables]
  [u]
    components = 2
  []
  [v]
    components = 2
  []
[]

[AuxVariables]
  [const]
    initial_condition = 0.5
  []
  [parsed]
    components = 2
  []
  [sum]
  []
[]

[Functions]
  [cosx]
    type = ParsedFunction
    expression = 'cos(x)'
  []
  [sinx]
    type = ParsedFunction
    expression = 'sin(x)'
  []
  [px]
    type = ParsedFunction
    expression = 'x'
  []
  [mx]
    type = ParsedFunction
    expression = '-x'
  []
[]

[ICs]
  [uic]
    type = ArrayFunctionIC
    variable = u
    function = 'cosx sinx'
  []
  [vic]
    type = ArrayFunctionIC
    variable = v
    function = 'px mx'
  []
[]

[AuxKernels]
  [parsed_aux]
    type = ArrayParsedAux
    variable = parsed
    expression = '(u^2 + v)*(x - const)*factor'
    coupled_array_variables = 'u v'
    coupled_variables = const
    constant_names = 'factor'
    constant_expressions = '3.14'
    use_xyzt = true
  []
  [sum_aux]
    type = ArrayVarReductionAux
    variable = sum
    array_variable = parsed
  []
[]

[Postprocessors]
  [avg]
    type = ElementAverageValue
    variable = sum
  []
[]

[Outputs]
  exodus = true
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
