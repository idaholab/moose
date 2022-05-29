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
  []
  [v]
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [reaction_u]
    type = Reaction
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [reaction_v]
    type = Reaction
    variable = v
  []
[]

[AuxVariables]
  [div]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [divergence]
    type = ADDivergenceAux
    variable = div
    u = u
    v = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [top]
    type = DirichletBC
    variable = v
    boundary = top
    value = 1
  []
  [bottom]
    type = DirichletBC
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
    type = SideIntegralVariablePostprocessor
    boundary = 'block_2_right'
    variable = 'u'
  []
  [s2]
    type = SideIntegralVariablePostprocessor
    boundary = 'block_2_left'
    variable = 'u'
  []
  [s3]
    type = SideIntegralVariablePostprocessor
    boundary = 'block_2_top'
    variable = 'v'
  []
  [s4]
    type = SideIntegralVariablePostprocessor
    boundary = 'block_2_bot'
    variable = 'v'
  []
[]

[Outputs]
  csv = true
  hide = 's1 s2 s3 s4'
[]
