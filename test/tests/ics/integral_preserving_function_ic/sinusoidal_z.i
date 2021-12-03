[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 20
  xmax = 1.5
  ymax = 1.7
  zmax = 1.9
  xmin = 0.0
  ymin = 0.0
  zmin = 0.0
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [power]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [power]
    type = IntegralPreservingFunctionIC
    variable = power
    magnitude = 550.0
    function = 'sin(pi * z / 1.9)'
    integral = vol
  []
[]

[Postprocessors]
  [vol]
    type = FunctionElementIntegral
    function = 'sin(pi * z / 1.9)'
    execute_on = 'initial'
  []
  [integrated_power] # should equal 550
    type = ElementIntegralVariablePostprocessor
    variable = power
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
