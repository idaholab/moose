[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 2
    ymax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.0 0 0'
    top_right = '1.0 1.0 0'
    block_id = 1
  []
  [corner_inward]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'corner_inward'
  []
  [corner_outward]
    input = corner_inward
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'corner_outward'
  []
[]

[Variables]
  [all_domain]
    type = MooseVariableFVReal
  []
  [part_domain]
    type = MooseVariableFVReal
    block = 1
  []
[]

[FVKernels]
  [diff_all]
    type = FVDiffusion
    variable = all_domain
    coeff = coeff
  []
  [diff_part]
    type = FVDiffusion
    variable = part_domain
    coeff = coeff
  []
[]

[FVBCs]
  # The boundaries where the flux kernels are executed are
  # the dirichlet BCs: left, right and corner_inward
  # On top and bottom, not executed because 0 flux is assumed
  [left]
    type = FVDirichletBC
    variable = all_domain
    boundary = left
    value = 2
  []
  [corner_inward]
    type = FVDirichletBC
    variable = all_domain
    boundary = right
    value = 1
  []
  [corner_outward]
    type = FVDirichletBC
    variable = part_domain
    boundary = corner_inward
    value = 2
  []
  [right]
    type = FVDirichletBC
    variable = part_domain
    boundary = left
    value = 1
  []
[]

[Materials]
  [diffusion]
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
  line_search = 'none'
[]

[Outputs]
  exodus = true
[]
