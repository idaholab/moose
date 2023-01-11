[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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
  [Markers]
    [marker]
      type = ErrorFractionMarker
      coarsen = 0.1
      indicator = error
      refine = 0.3
    []
  []
[]

[Outputs]
  exodus = true
[]
