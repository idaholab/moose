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
[]

[AuxVariables]
  [v]
    components = 8
  []
[]

[ICs]
  [uic]
    type = ArrayConstantIC
    variable = u
    value = '0.1 3'
  []
  [vic]
    type = ArrayConstantIC
    variable = v
    value = '2 6 9 7 1.1 2 5 4'
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
