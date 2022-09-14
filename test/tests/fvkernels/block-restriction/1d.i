[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [left_right]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'left_right'
  []
  [right_left]
    input = left_right
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'right_left'
  []
[]

[Variables]
  [left]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    block = 0
  []
  [right]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    block = 1
  []
[]

[FVKernels]
  [left]
    type = FVDiffusion
    variable = left
    coeff = coeff_left
    block = 0
    coeff_interp_method = average
  []
  [right]
    type = FVDiffusion
    variable = right
    coeff = coeff_right
    block = 1
    coeff_interp_method = average
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = left
    boundary = left
    value = 0
  []
  [left_right]
    type = FVDirichletBC
    variable = left
    boundary = left_right
    value = 1
  []
  [right_left]
    type = FVDirichletBC
    variable = right
    boundary = right_left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = right
    boundary = right
    value = 1
  []
[]

[Materials]
  [left]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff_left'
    prop_values = '1'
    block = 0
  []
  [right]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff_right'
    prop_values = '1'
    block = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = 'none'
[]

[Outputs]
  exodus = true
[]
