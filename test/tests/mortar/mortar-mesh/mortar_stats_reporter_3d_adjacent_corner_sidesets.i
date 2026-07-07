[Mesh]
  [secondary_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    elem_type = HEX20
  []
  [secondary_sidesets]
    type = RenameBoundaryGenerator
    input = secondary_block
    old_boundary = '0 1 2 3 4 5'
    new_boundary = 'secondary_bottom secondary_back secondary_right secondary_front secondary_left secondary_top'
  []
  [secondary_block_id]
    type = SubdomainIDGenerator
    input = secondary_sidesets
    subdomain_id = 1
  []

  [primary_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
    elem_type = HEX20
  []
  [primary_sidesets]
    type = RenameBoundaryGenerator
    input = primary_block
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '100 101 102 103 104 105'
  []
  [primary_block_id]
    type = SubdomainIDGenerator
    input = primary_sidesets
    subdomain_id = 2
  []

  [combined]
    type = MeshCollectionGenerator
    inputs = 'secondary_block_id primary_block_id'
  []
  [primary_sidesets_rename]
    type = RenameBoundaryGenerator
    input = combined
    old_boundary = '100 101 102 103 104 105'
    new_boundary = 'primary_bottom primary_back primary_right primary_front primary_left primary_top'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = primary_sidesets_rename
    old_block = '1 2'
    new_block = 'secondary_volume primary_volume'
  []
  [secondary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = block_rename
    sidesets = 'secondary_right secondary_top'
    new_block_id = '12'
    new_block_name = 'secondary_lower'
  []
  [primary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = secondary_lower
    sidesets = 'primary_right primary_top'
    new_block_id = '11'
    new_block_name = 'primary_lower'
  []
[]

[Variables]
  [T]
    block = 'secondary_volume primary_volume'
    order = SECOND
  []
  [lambda]
    block = 'secondary_lower'
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = T
    block = 'secondary_volume primary_volume'
  []
[]

[Constraints]
  [mortar]
    type = EqualValueConstraint
    primary_boundary = 'primary_right primary_top'
    secondary_boundary = 'secondary_right secondary_top'
    primary_subdomain = '11'
    secondary_subdomain = '12'
    variable = lambda
    secondary_variable = T
    debug_mesh = true
    segment_quadrature = FIFTH
  []
[]

[Problem]
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
