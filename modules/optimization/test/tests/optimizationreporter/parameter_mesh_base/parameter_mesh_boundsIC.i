[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
  second_order = true
  parallel_type = REPLICATED
[]

[Problem]
  solve=false
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
  [lowerBoundsSecond]
    family = LAGRANGE
    order = SECOND
  []
  [upperBoundsSecond]
    family = LAGRANGE
    order = SECOND
  []
  [initialConditionsSecond]
    family = LAGRANGE
    order = SECOND
  []
  [elemLowerBounds]
    family = MONOMIAL
    order = CONSTANT
  []
  [elemUpperBounds]
    family = MONOMIAL
    order = CONSTANT
  []
  [elemInitialConditions]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [lowerBounds_aux]
    type = ParsedAux
    use_xyzt = true
    variable = lowerBounds
    expression = 't*x'
  []
  [upperBounds_aux]
    type = ParsedAux
    use_xyzt = true
    variable = upperBounds
    expression = '2+t*x'
  []
  [initialConditions_aux]
    type = ParsedAux
    use_xyzt = true
    variable = initialConditions
    expression = 'x'
  []
  [lowerBoundsSecond_aux]
    type = ParsedAux
    use_xyzt = true
    variable = lowerBoundsSecond
    expression = 't*y'
  []
  [upperBoundsSecond_aux]
    type = ParsedAux
    use_xyzt = true
    variable = upperBoundsSecond
    expression = '2+t*y'
  []
  [initialConditionsSecond_aux]
    type = ParsedAux
    use_xyzt = true
    variable = initialConditionsSecond
    expression = 'y'
  []
  [elemLowerBounds_aux]
    type = ParsedAux
    use_xyzt = true
    variable = elemLowerBounds
    expression = 't*x'
  []
  [elemUpperBounds_aux]
    type = ParsedAux
    use_xyzt = true
    variable = elemUpperBounds
    expression = '2+t*x'
  []
  [elemInitialConditions_aux]
    type = ParsedAux
    use_xyzt = true
    variable = elemInitialConditions
    expression = 'x'
  []
[]

[BCs]
[]

[Executioner]
  type = Transient
  # time control
  start_time = 0.0
  dt = 1.0
  end_time = 3.0
[]

[Outputs]
  exodus = true
[]
