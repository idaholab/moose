[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
    elem_type = QUAD4
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3'
    new_boundary = 'lb_bottom lb_right lb_top lb_left'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  []
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 3
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
    elem_type = QUAD4
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block
    subdomain_id = 2
  []
  [right_block_change_boundary_id]
    type = RenameBoundaryGenerator
    input = right_block_id
    old_boundary = '0 1 2 3'
    new_boundary = '100 101 102 103'
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_change_boundary_id'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = combined
    old_block = '1 2'
    new_block = 'left_block right_block'
  []
  [right_right_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = block_rename
    new_boundary = rb_right
    block = right_block
    normal = '1 0 0'
  []
  [right_left_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_right_sideset
    new_boundary = rb_left
    block = right_block
    normal = '-1 0 0'
  []
  [right_top_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_left_sideset
    new_boundary = rb_top
    block = right_block
    normal = '0 1 0'
  []
  [right_bottom_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = right_top_sideset
    new_boundary = rb_bottom
    block = right_block
    normal = '0 -1 0'
  []
  [secondary]
    input = right_bottom_sideset
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'lb_right'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'rb_left'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []
[]

[Variables]
  [T]
    block = 'left_block right_block'
    type = MooseVariableFVReal
  []
  [lambda]
    block = 'secondary_lower'
    family = MONOMIAL
    order = CONSTANT
  []
[]

[FVBCs]
  [neumann]
    type = FVFunctionDirichletBC
    function = exact_soln_primal
    variable = T
    boundary = 'lb_bottom lb_top lb_left rb_bottom rb_right rb_top'
  []
[]

[FVKernels]
  [conduction]
    type = FVDiffusion
    variable = T
    block = 'left_block right_block'
    coeff = 1
  []
  [sink]
    type = FVReaction
    variable = T
    block = 'left_block right_block'
  []
  [forcing_function]
    type = FVBodyForce
    variable = T
    function = forcing_function
    block = 'left_block right_block'
  []
[]

[Functions]
  [forcing_function]
    type = ParsedFunction
    expression = ''
  []
  [exact_soln_primal]
    type = ParsedFunction
    expression = ''
  []
  [exact_soln_lambda]
    type = ParsedFunction
    expression = ''
  []
  [mms_secondary]
    type = ParsedFunction
    expression = ''
  []
  [mms_primary]
    type = ParsedFunction
    expression = ''
  []
[]

[Constraints]
  [mortar]
    type = GapHeatConductanceTest
    primary_boundary = rb_left
    secondary_boundary = lb_right
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    secondary_variable = T
    variable = lambda
    secondary_gap_conductance = 1
    primary_gap_conductance = 1
    secondary_mms_function = mms_secondary
    primary_mms_function = mms_primary
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [L2lambda]
    type = ElementL2Error
    variable = lambda
    function = exact_soln_lambda
    execute_on = 'timestep_end'
    block = 'secondary_lower'
  []
  [L2u]
    type = ElementL2Error
    variable = T
    function = exact_soln_primal
    execute_on = 'timestep_end'
    block = 'left_block right_block'
  []
  [h]
    type = AverageElementSize
    block = 'left_block right_block'
  []
[]
