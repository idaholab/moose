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
    value = '1 + t'
  []
  [fcn_01]
    type = ParsedFunction
    value = '4 + t'
  []
  [fcn_02]
    type = ParsedFunction
    value = '7 + t'
  []
  [fcn_10]
    type = ParsedFunction
    value = '2 + t'
  []
  [fcn_11]
    type = ParsedFunction
    value = '5 + t'
  []
  [fcn_12]
    type = ParsedFunction
    value = '8 + t'
  []
  [fcn_20]
    type = ParsedFunction
    value = '3 + t'
  []
  [fcn_21]
    type = ParsedFunction
    value = '6 + t'
  []
  [fcn_22]
    type = ParsedFunction
    value = '9 + t'
  []
[]

[Materials]
  [./tensor]
    type = ADGenericFunctionRankTwoTensor
    tensor_name = function
    tensor_functions = 'fcn_00 fcn_01 fcn_02 fcn_10 fcn_11 fcn_12 fcn_20 fcn_21 fcn_22'
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
