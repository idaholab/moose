[Mesh]
  [bulk]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.75 2.0'
    dy = '0.5 0.3 0.025 3.6 0.025 0.3 0.5'
    ix = '5 8 20'
    iy = '5 3 1 36 1 3 5'
    # bottom subdomain ids
    subdomain_id = '
      0 0 0
      0 2 1
      0 2 3
      0 2 4
      0 2 3
      0 1 1
      0 0 0
    '
    # top subdomain ids
  []
  [hollow_concrete]
    type = BlockDeletionGenerator
    input = bulk
    block = 4
  []
  [rename_blocks]
    type = RenameBlockGenerator
    input = hollow_concrete
    old_block = '0 1 2 3'
    new_block = 'concrete_hd concrete water Al'
  []
  [add_concrete_outer_boundary]
    type = RenameBoundaryGenerator
    input = rename_blocks
    old_boundary = 'left right bottom top'
    new_boundary = 'air_boundary symmetry ground air_boundary'
  []
  [add_water_concrete_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_concrete_outer_boundary
    primary_block = 'water water water'
    paired_block = 'concrete_hd concrete Al'
    new_boundary = 'water_boundary'
  []
  [add_water_concrete_interface_inwards]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_water_concrete_interface
    primary_block = 'concrete_hd concrete Al'
    paired_block = 'water water water'
    new_boundary = 'water_boundary_inwards'
  []
  [add_water_bottom]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_water_concrete_interface_inwards
    primary_block = water
    paired_block = concrete_hd
    normal = '0 -1 0'
    new_boundary = water_bottom
    replace = true
  []
  [add_inner_cavity_solid]
    type = SideSetsAroundSubdomainGenerator
    input = add_water_bottom
    block = Al
    new_boundary = 'inner_cavity_solid'
    include_only_external_sides = true
  []
  [add_inner_cavity_water]
    type = SideSetsAroundSubdomainGenerator
    input = add_inner_cavity_solid
    block = water
    new_boundary = 'inner_cavity_water'
    include_only_external_sides = true
  []
[]
