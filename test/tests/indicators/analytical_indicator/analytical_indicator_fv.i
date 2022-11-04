[Mesh]
  [mesh]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 20
  ny = 1
  []
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[Functions]
  [solution]
    type = ParsedFunction
    expression = (exp(x)-1)/(exp(1)-1)
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = coeff
  []
  [conv]
    type = FVAdvection
    variable = u
    velocity = '1 0 0'
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Adaptivity]
  [Indicators]
    [error]
      type = AnalyticalIndicator
      variable = u
      function = solution
    []
  []
[]

[Outputs]
  exodus = true
[]
