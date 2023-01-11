[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD9
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[AuxVariables]
  [x]
    components = 2
  []
  [y]
    order = SECOND
    components = 2
  []
  [z]
    family = MONOMIAL
    order = SIXTH
    components = 3
  []
[]

[Functions]
  [func1]
    type = ParsedFunction
    expression = x*x+y*y
  []

  [func2]
    type = ParsedFunction
    expression = t*((x*x)+(y*y))
  []

  [func3]
    type = ParsedFunction
    expression = t
  []
[]

[AuxKernels]
  [x]
    type = FunctionArrayAux
    variable = x
    functions = 'func1 func2'
  []
  [y]
    type = FunctionArrayAux
    variable = y
    functions = 'func1 func2'
  []
  [z]
    type = FunctionArrayAux
    variable = z
    functions = 'func1 func2 func3'
  []
[]

[Postprocessors]
  [x0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = x
    component = 0
  []
  [x1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = x
    component = 1
  []
  [y0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = y
    component = 0
  []
  [y1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = y
    component = 1
  []
  [z0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = z
    component = 0
  []
  [z1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = z
    component = 1
  []
  [z2]
    type = ElementIntegralArrayVariablePostprocessor
    variable = z
    component = 2
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  num_steps = 5
  dt = 1
[]

[Outputs]
  exodus = true
[]
