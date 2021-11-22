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
    coeff = coeff_u
  []
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = coeff_v
  []
[]

[FVBCs]
  [left_u]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff_u coeff_v'
    prop_values = '1      1e-20'
  []
[]

[Executioner]
  type = Steady
  petsc_options = '-pc_svd_monitor'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'svd'
  automatic_scaling = true
  verbose = true
[]

[Outputs]
  exodus = true
[]
