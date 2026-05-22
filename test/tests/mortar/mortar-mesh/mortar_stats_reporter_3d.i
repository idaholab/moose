[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 2
    nz = 2
    xmin = 0
    xmax = 0.3
    ymin = 0
    ymax = .5
    zmin = 0
    zmax = .5
    elem_type = TET4
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3 4 5'
    new_boundary = 'lb_bottom lb_back lb_right lb_front lb_left lb_top'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  []
  [right_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 3
    nz = 3
    xmin = 0.3
    xmax = 0.6
    ymin = 0
    ymax = .5
    zmin = 0
    zmax = .5
    elem_type = TET4
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block
    subdomain_id = 2
  []
  [right_block_change_boundary_id]
    type = RenameBoundaryGenerator
    input = right_block_id
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '100 101 102 103 104 105'
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
  [secondary]
    input = right_left_sideset
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'lb_right'
    new_block_id = '12'
    new_block_name = 'secondary'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'rb_left'
    new_block_id = '11'
    new_block_name = 'primary'
  []
[]

[Variables]
  [T]
    block = '1 2'
    order = FIRST
  []
  [lambda]
    block = 'secondary'
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  []
[]

[Constraints]
  [mortar]
    type = EqualValueConstraint
    primary_boundary = 'rb_left'
    secondary_boundary = 'lb_right'
    primary_subdomain = '11'
    secondary_subdomain = '12'
    variable = lambda
    secondary_variable = T
    debug_mesh = true
  []
[]

[Problem]
  # kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters]
  [mortar_stats]
    type = MortarSegmentMeshReporter
    execute_on = INITIAL
    use_displaced_mesh = false
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = NONE
    execute_on = INITIAL
  []
[]
