[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Functions]
  [linear]
    type = ParsedFunction
    expression = 'x + 10*t'
  []
[]

[GlobalParams]
  family = MONOMIAL
  order = CONSTANT
[]

[AuxVariables]
  [base_variable]
  []
[]

[AuxKernels]
  [base]
    type = FunctionAux
    function = 'linear'
    variable = 'base_variable'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [v_current]
    type = SingleInternalFaceValue
    variable = 'base_variable'
    state = current
    element_id = 0
    side_index = 1
  []
  [v_old]
    type = SingleInternalFaceValue
    variable = 'base_variable'
    state = old
    element_id = 0
    side_index = 1
  []
  [v_older]
    type = SingleInternalFaceValue
    variable = 'base_variable'
    state = older
    element_id = 0
    side_index = 1
  []
[]

[Outputs]
  csv = true
[]
