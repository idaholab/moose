# Tests the ElementValueSampler vector post-processor. In this test, 2 constant
# monomial variables are given distributions by a function and are output to a CSV file.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Functions]
  [u_fn]
    type = ParsedFunction
    expression = '2 * x + 3 * y'
  []
  [v_fn]
    type = ParsedFunction
    expression = 'x + y'
  []
[]

[AuxVariables]
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
  [v]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    variable = u
    function = u_fn
  []
  [v_ic]
    type = FunctionIC
    variable = v
    function = v_fn
  []
[]

[VectorPostprocessors]
  [element_value_sampler]
    type = ElementValueSampler
    variable = 'u v'
    sort_by = id
    execute_on = 'initial'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = 'element_value_sampler'
  csv = true
  execute_on = 'initial'
[]
