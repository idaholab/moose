[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[FVKernels]
  [diff_u]
    type = FVDiffusion
    variable = u
    coeff = coeff
  []
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
[]

[FVBCs]
  [left_u]
    type = FVNeumannBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 42
  []
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

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
