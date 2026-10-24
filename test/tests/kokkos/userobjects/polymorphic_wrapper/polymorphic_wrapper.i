[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[AuxVariables]
  [result]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[UserObjects]
  [affine]
    type = KokkosAffineUserObject
    factor = 2
    offset = 1
  []
  [quadratic]
    type = KokkosQuadraticUserObject
    factor = 4
    coefficient = 3
    offset = 2
  []
[]

[AuxKernels]
  [result]
    type = KokkosPolymorphicUserObjectAux
    variable = result
    first = affine
    second = quadratic
    execute_on = initial
  []
[]

[Postprocessors]
  [result]
    type = ElementAverageValue
    variable = result
    execute_on = initial
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
