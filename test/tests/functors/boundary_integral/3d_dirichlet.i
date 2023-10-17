[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 4
  nz = 2
  xmax = 2
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 7
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 42
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Postprocessors]
  [reference_top]
    type = SideIntegralVariablePostprocessor
    variable = 'v'
    boundary = 'top'
  []
  [reference_left]
    type = SideIntegralVariablePostprocessor
    variable = 'v'
    boundary = 'left'
  []
  [computed_top]
    type = BoundaryFunctorIntegralOutput
    variable = 'v'
    boundary = 'top'
  []
  [computed_left]
    type = BoundaryFunctorIntegralOutput
    variable = 'v'
    boundary = 'left'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  residual_and_jacobian_together = true
[]

[Outputs]
  csv = true
[]
