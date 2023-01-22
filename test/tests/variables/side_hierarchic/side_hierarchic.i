[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  elem_type = QUAD9
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [./side_var]
    order = CONSTANT
    family = SIDE_HIERARCHIC
  [../]
[]

[AuxVariables]
  [./aux_side_var]
    order = FIRST
    family = SIDE_HIERARCHIC
  [../]
[]


[Functions]
  [./nl_var]
    type = ParsedFunction
    expression = 'x+y+1'
  [../]
  [./aux_var]
    type = ParsedFunction
    expression = 'x-y+10'
  [../]
[]

[ICs]
  [./side_nl]
    type = FunctionIC
    variable = side_var
    function = nl_var
  [../]
  [./side_aux]
    type = FunctionIC
    variable = aux_side_var
    function = aux_var
  [../]
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
[]
