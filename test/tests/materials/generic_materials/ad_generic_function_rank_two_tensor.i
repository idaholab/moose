[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [fcn_00]
    type = ParsedFunction
    expression = '1 + t'
  []
  [fcn_10]
    type = ParsedFunction
    expression = '4 + t'
  []
  [fcn_20]
    type = ParsedFunction
    expression = '7 + t'
  []
  [fcn_01]
    type = ParsedFunction
    expression = '2 + t'
  []
  [fcn_11]
    type = ParsedFunction
    expression = '5 + t'
  []
  [fcn_21]
    type = ParsedFunction
    expression = '8 + t'
  []
  [fcn_02]
    type = ParsedFunction
    expression = '3 + t'
  []
  [fcn_12]
    type = ParsedFunction
    expression = '6 + t'
  []
  [fcn_22]
    type = ParsedFunction
    expression = '9 + t'
  []
[]

[Materials]
  [./tensor]
    type = ADGenericFunctionRankTwoTensor
    tensor_name = function
    # tensor values are column major-ordered
    tensor_functions = 'fcn_00 fcn_10 fcn_20 fcn_01 fcn_11 fcn_21 fcn_02 fcn_12 fcn_22'
    outputs = all
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Postprocessors]
  [00]
    type = ElementAverageValue
    variable = function_00
  []
  [01]
    type = ElementAverageValue
    variable = function_01
  []
  [02]
    type = ElementAverageValue
    variable = function_02
  []
  [10]
    type = ElementAverageValue
    variable = function_10
  []
  [11]
    type = ElementAverageValue
    variable = function_11
  []
  [12]
    type = ElementAverageValue
    variable = function_12
  []
  [20]
    type = ElementAverageValue
    variable = function_20
  []
  [21]
    type = ElementAverageValue
    variable = function_21
  []
  [22]
    type = ElementAverageValue
    variable = function_22
  []
[]

[Outputs]
  csv = true
[]
