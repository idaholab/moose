[Mesh]
  [circle]
    type = FileMeshGenerator
    file = circle.msh
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
    boundary = outer
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
    boundary = outer
  []
[]
