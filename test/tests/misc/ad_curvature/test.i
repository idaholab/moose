[Mesh]
  [sphere]
    type = SphereMeshGenerator
    radius = 1
    nr = 1
    elem_type = HEX27
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [curvature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [curvature]
    variable = curvature
    type = ADCurvatureAux
    execute_on = 'initial'
    boundary = 0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [average]
    type = SideAverageValue
    execute_on = 'initial'
    variable = curvature
    boundary = 0
  []
[]
