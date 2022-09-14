[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
    xmax = 4
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '2.0 0 0'
    block_id = 1
    top_right = '4.0 1.0 0'
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
  [fv]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = fv
    coeff = diff
    coeff_interp_method = average
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = fv
    boundary = left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = fv
    boundary = right
    value = 1
  []
[]

[Materials]
  [left]
    type = ADGenericFunctorMaterial
    prop_names = 'diff'
    prop_values = '1'
    block = 0
  []
  [right]
    type = ADGenericFunctorMaterial
    prop_names = 'diff'
    prop_values = '2'
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
