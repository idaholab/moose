[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.75 0.5 0.75'
    dy = '2'
    ix = '4 20 4'
    iy = '5'
  []
  [move]
    type = TransformGenerator
    input = cmg
    transform = TRANSLATE_CENTER_ORIGIN
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[AuxVariables]
  [lowerBounds]
    family = LAGRANGE
    order = FIRST
  []
  [upperBounds]
    family = LAGRANGE
    order = FIRST
  []
  [initialConditions]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [initialConditions_aux]
    type = ParsedAux
    variable = initialConditions
    expression = '50'
  []

  [lowerBounds_aux]
    type = ParsedAux
    variable = lowerBounds
    expression = '.1'
  []
  [upperBounds_aux]
    type = ParsedAux
    variable = upperBounds
    expression = '100'
  []
[]

[Outputs]
  [in]
    type = Exodus
  []
[]
