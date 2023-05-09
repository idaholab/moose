[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
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
    type = FVBoundaryIntegralValueConstraint
    variable = v
    boundary = right
    phi0 = 42
    lambda = lambda
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 7
  []
  [right]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 42
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
  hide = lambda
[]
