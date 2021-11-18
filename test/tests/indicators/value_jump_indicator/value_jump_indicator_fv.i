[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Adaptivity]
  [Indicators]
    [error]
      type = ValueJumpIndicator
      variable = something
    []
  []
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[ICs]
  [leftright]
    type = BoundingBoxIC
    variable = something
    inside = 1
    y2 = 1
    y1 = 0
    x2 = 0.5
    x1 = 0
  []
[]

[AuxVariables]
  [something]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = coeff
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Outputs]
  exodus = true
[]
