[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 1.0
    ymin = -1.0
    ymax = 1.0
    nx = 2
    ny = 2
    elem_type = QUAD4
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  []
  [left]
    type = LowerDBlockFromSidesetGenerator
    input = left_block_id
    sidesets = '13'
    new_block_id = '10003'
    new_block_name = 'secondary_left'
  []
  [right]
    type = LowerDBlockFromSidesetGenerator
    input = left
    sidesets = '11'
    new_block_id = '10001'
    new_block_name = 'primary_right'
  []
  [bottom]
    type = LowerDBlockFromSidesetGenerator
    input = right
    sidesets = '10'
    new_block_id = '10000'
    new_block_name = 'secondary_bottom'
  []
  [top]
    type = LowerDBlockFromSidesetGenerator
    input = bottom
    sidesets = '12'
    new_block_id = '10002'
    new_block_name = 'primary_top'
  []

  [corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = top
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [epsilon]
    order = SECOND
    family = SCALAR
  []
  [sigma]
    order = SECOND
    family = SCALAR
  []
[]

[Kernels]
  [diff1]
    type = Diffusion
    variable = u
    block = 1
  []
[]

[Problem]
  kernel_coverage_check = false
  error_on_jacobian_nonzero_reallocation = true
[]

[BCs]
  [fix_right]
    type = DirichletBC
    variable = u
    boundary = pinned_node
    value = 0
  []
[]

[Constraints]
  [mortarlr]
    type = PenaltyEqualValueConstraint
    primary_boundary = '11'
    secondary_boundary = '13'
    primary_subdomain = 'primary_right'
    secondary_subdomain = 'secondary_left'
    secondary_variable = u
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [periodiclr]
    type = PenaltyPeriodicSegmentalConstraint
    primary_boundary = '11'
    secondary_boundary = '13'
    primary_subdomain = 'primary_right'
    secondary_subdomain = 'secondary_left'
    secondary_variable = u
    epsilon = epsilon
    sigma = sigma
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [mortarbt]
    type = PenaltyEqualValueConstraint
    primary_boundary = '12'
    secondary_boundary = '10'
    primary_subdomain = 'primary_top'
    secondary_subdomain = 'secondary_bottom'
    secondary_variable = u
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [periodicbt]
    type = PenaltyPeriodicSegmentalConstraint
    primary_boundary = '12'
    secondary_boundary = '10'
    primary_subdomain = 'primary_top'
    secondary_subdomain = 'secondary_bottom'
    secondary_variable = u
    epsilon = epsilon
    sigma = sigma
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]
