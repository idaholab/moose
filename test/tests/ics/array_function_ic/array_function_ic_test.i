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
  [u]
    components = 2
  []
  [u0]
  []
  [u1]
  []
[]

[AuxVariables]
  [v]
    components = 2
  []
[]

[Functions]
  [sinx]
    type = ParsedFunction
    expression = sin(x)
  []
  [siny]
    type = ParsedFunction
    expression = sin(y)
  []
[]

[ICs]
  [uic]
    type = ArrayFunctionIC
    variable = u
    function = 'sinx siny'
  []
  [u0ic]
    type = FunctionIC
    variable = u0
    function = sinx
  []
  [u1ic]
    type = FunctionIC
    variable = u1
    function = siny
  []
  [vic]
    type = ArrayFunctionIC
    variable = v
    function = 'sinx siny'
  []
[]

[Postprocessors]
  [uint0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 0
  []
  [uint1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 1
  []
  [u0int]
    type = ElementIntegralVariablePostprocessor
    variable = u0
  []
  [u1int]
    type = ElementIntegralVariablePostprocessor
    variable = u1
  []
  [vint0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = v
    component = 0
  []
  [vint1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = v
    component = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
