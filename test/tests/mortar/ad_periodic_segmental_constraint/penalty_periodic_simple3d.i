[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -3.0
    xmax = 3.0
    ymin = -3.0
    ymax = 3.0
    zmin = -3.0
    zmax = 3.0
    nx = 3
    ny = 3
    nz = 3
    elem_type = HEX8
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '10 11 12 13 14 15'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  []
  [left]
    type = LowerDBlockFromSidesetGenerator
    input = left_block_id
    sidesets = '14'
    new_block_id = '10004'
    new_block_name = 'secondary_left'
  []
  [right]
    type = LowerDBlockFromSidesetGenerator
    input = left
    sidesets = '12'
    new_block_id = '10002'
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
    sidesets = '15'
    new_block_id = '10005'
    new_block_name = 'primary_top'
  []
  [back]
    type = LowerDBlockFromSidesetGenerator
    input = top
    sidesets = '11'
    new_block_id = '10001'
    new_block_name = 'secondary_back'
  []
  [front]
    type = LowerDBlockFromSidesetGenerator
    input = back
    sidesets = '13'
    new_block_id = '10003'
    new_block_name = 'primary_front'
  []

  [corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = front
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [epsilon]
    order = THIRD
    family = SCALAR
  []
[]

[AuxVariables]
  [sigma]
    order = THIRD
    family = SCALAR
  []
[]

[AuxScalarKernels]
  [sigma]
    type = FunctionScalarAux
    variable = sigma
    function = '1 2 3'
    execute_on = initial #timestep_end
  []
[]

[Kernels]
  [diff1]
    type = ADDiffusion
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
    type = ADPenaltyEqualValueConstraint
    primary_boundary = '12'
    secondary_boundary = '14'
    primary_subdomain = 'primary_right'
    secondary_subdomain = 'secondary_left'
    secondary_variable = u
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [periodiclr]
    type = ADPenaltyPeriodicSegmentalConstraint
    primary_boundary = '12'
    secondary_boundary = '14'
    primary_subdomain = 'primary_right'
    secondary_subdomain = 'secondary_left'
    secondary_variable = u
    epsilon = epsilon
    sigma = sigma
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [mortarbt]
    type = ADPenaltyEqualValueConstraint
    primary_boundary = '15'
    secondary_boundary = '10'
    primary_subdomain = 'primary_top'
    secondary_subdomain = 'secondary_bottom'
    secondary_variable = u
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [periodicbt]
    type = ADPenaltyPeriodicSegmentalConstraint
    primary_boundary = '15'
    secondary_boundary = '10'
    primary_subdomain = 'primary_top'
    secondary_subdomain = 'secondary_bottom'
    secondary_variable = u
    epsilon = epsilon
    sigma = sigma
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [mortarbf]
    type = ADPenaltyEqualValueConstraint
    primary_boundary = '13'
    secondary_boundary = '11'
    primary_subdomain = 'primary_front'
    secondary_subdomain = 'secondary_back'
    secondary_variable = u
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [periodicbf]
    type = ADPenaltyPeriodicSegmentalConstraint
    primary_boundary = '13'
    secondary_boundary = '11'
    primary_subdomain = 'primary_front'
    secondary_subdomain = 'secondary_back'
    secondary_variable = u
    epsilon = epsilon
    sigma = sigma
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
[]

[Preconditioning]
  [smp]
    full = true
    type = SMP
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  solve_type = NEWTON
[]

[Outputs]
#  exodus = true
  csv = true
[]
