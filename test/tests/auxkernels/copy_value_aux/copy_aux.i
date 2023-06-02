[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 3.141
  ymin = 0
  ymax = 3.141
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [w]
    family = MONOMIAL
    order = CONSTANT
  []
  [u_copy]
  []
  [w_copy]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    variable = 'u'
    function = parsed_function
  []
  [w_ic]
    type = FunctionIC
    variable = 'w'
    function = 'x + y'
  []
[]

[Functions]
  [parsed_function]
    type = ParsedFunction
    expression = 'sin(x)-cos(y/2)'
  []
[]

[AuxKernels]
  [copy_u]
    type = CopyValueAux
    variable = u_copy
    source = u
  []
  [copy_w]
    type = CopyValueAux
    variable = w_copy
    source = w
  []
[]

[VectorPostprocessors]
  [results]
    type = LineValueSampler
    start_point = '0.0001 0.99 0'
    end_point = '3.14 0.99 0'
    variable = 'u w u_copy w_copy'
    num_points = 17
    sort_by = x
  []
[]

[Problem]
  solve = false
[]
[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
