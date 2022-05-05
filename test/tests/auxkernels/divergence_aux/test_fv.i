[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.5 1 0.1'
    dy = '1.3 1 0.9'
    ix = '2 4 1'
    iy = '2 3 3'
    subdomain_id = '1 1 1
                    1 2 1
                    1 1 1'
  []
  [add_inner_boundaries_top]
    type = SideSetsAroundSubdomainGenerator
    input = cmg
    new_boundary = 'block_2_top'
    block = 2
    normal = '0 1 0'
  []
  [add_inner_boundaries_bot]
    type = SideSetsAroundSubdomainGenerator
    input = add_inner_boundaries_top
    new_boundary = 'block_2_bot'
    block = 2
    normal = '0 -1 0'
  []
  [add_inner_boundaries_right]
    type = SideSetsAroundSubdomainGenerator
    input = add_inner_boundaries_bot
    new_boundary = 'block_2_right'
    block = 2
    normal = '1 0 0'
  []
  [add_inner_boundaries_left]
    type = SideSetsAroundSubdomainGenerator
    input = add_inner_boundaries_right
    new_boundary = 'block_2_left'
    block = 2
    normal = '-1 0 0'
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
  [v]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [diff_u]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
  [reaction_u]
    type = FVReaction
    variable = u
  []
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = 2
  []
  [reaction_v]
    type = FVReaction
    variable = v
  []
[]

[AuxVariables]
  [div]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [divergence]
    type = ADDivergenceAux
    variable = div
    u = 'u'
    v = 'v'
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 2
  []
  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [top]
    type = FVDirichletBC
    variable = v
    boundary = top
    value = 2
  []
  [bottom]
    type = FVDirichletBC
    variable = v
    boundary = bottom
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [int_divergence]
    type = ElementL1Error
    block = 2
    variable = div
    function = 0
  []
  [sum_surface_current]
    type = ParsedPostprocessor
    function = 's1 - s2 + s3 - s4'
    pp_names = 's1 s2 s3 s4'
  []
  [s1]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'block_2_right'
    functor = 'u'
  []
  [s2]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'block_2_left'
    functor = 'u'
  []
  [s3]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'block_2_top'
    functor = 'v'
  []
  [s4]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'block_2_bot'
    functor = 'v'
  []
[]

[Outputs]
  csv = true
  hide = 's1 s2 s3 s4'
[]
