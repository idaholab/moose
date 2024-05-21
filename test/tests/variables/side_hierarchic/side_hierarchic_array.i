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
  [side_var]
    order = ELEVENTH
    family = SIDE_HIERARCHIC
    components = 5
  []
[]

[AuxVariables]
  [aux_side_var]
    order = FOURTH
    family = SIDE_HIERARCHIC
    components = 4
  []
[]

[Functions]
  [nl_var]
    type = ParsedFunction
    expression = 'x+y+1'
  []
  [aux_var]
    type = ParsedFunction
    expression = 'x-y+10'
  []
[]

[ICs]
  [side_nl]
    type = ArrayFunctionIC
    variable = side_var
    function = 'nl_var nl_var nl_var nl_var nl_var'
  []
  [side_aux]
    type = ArrayFunctionIC
    variable = aux_side_var
    function = 'aux_var aux_var aux_var aux_var'
  []
[]

[Outputs]
  [out]
    type = Exodus
    side_discontinuous = true
    discontinuous = true
  []
[]

[Executioner]
  type = Steady
[]
