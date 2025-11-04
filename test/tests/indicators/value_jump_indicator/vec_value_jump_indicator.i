[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Adaptivity]
  [./Indicators]
    [./error]
      type = VectorValueJumpIndicator
      variable = something
    [../]
  [../]
[]


[ICs]
  [./leftright_1]
    type = BoundingBoxIC
    variable = something_1
    inside = 1
    y2 = 0.5
    y1 = 0
    x2 = 0.5
    x1 = 0
  [../]
[]

[AuxVariables]
  [./something]
    type = VectorMooseVariable
    order = CONSTANT
    family = MONOMIAL_VEC
  [../]
  [./something_1]
    order = CONSTANT
    family = MONOMIAL
    outputs = 'none'
  [../]
[]

[AuxKernels]
  [something]
    type = ParsedVectorAux
    variable = something
    coupled_variables = 'something_1'
    expression_x = 'something_1'
    expression_y = '0.0'
  []
[]


[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
