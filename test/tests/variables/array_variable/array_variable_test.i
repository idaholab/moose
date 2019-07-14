[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 8
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    components = 4
    initial_condition = '1 2 3 4'
  [../]
  [./uu]
    order = FIRST
    family = LAGRANGE
    components = 2
    initial_condition = '1 2'
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
    components = 2
    initial_condition = '5 6'
  [../]
  [./w]
    order = CONSTANT
    family = MONOMIAL
    components = 3
    initial_condition = '7 8 9'
  [../]
  [./x]
    order = THIRD
    family = MONOMIAL
    components = 2
    initial_condition = '10 11'
  [../]
  [./y]
    order = FIRST
    family = L2_LAGRANGE
    components = 3
    initial_condition = '12 13 14'
  [../]
[]

[Postprocessors]
  [u0int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 0
  []
  [u1int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 1
  []
  [u2int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 2
  []
  [u3int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 3
  []
  [uu0int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = uu
    component = 0
  []
  [uu1int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = uu
    component = 1
  []
  [v0int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = v
    component = 0
  []
  [v1int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = v
    component = 1
  []
  [w0int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = w
    component = 0
  []
  [w1int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = w
    component = 1
  []
  [w2int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = w
    component = 2
  []
  [x0int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = x
    component = 0
  []
  [x1int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = x
    component = 1
  []
  [y0int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = y
    component = 0
  []
  [y1int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = y
    component = 1
  []
  [y2int]
    type = ElementIntegralArrayVariablePostprocessor
    variable = y
    component = 2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
