[Mesh]
  [bulk]
    type = CartesianMeshGenerator
    dim = 3
    dx = '2 1 0.025 4.0 0.025 1 2'
    dy = '2 1 0.025 7.5 0.025 1 2'
    dz = '1 1 0.025 3.5 0.025 1 3'
    ix = '4 2 1  8 1 2 4'
    iy = '4 2 1 15 1 2 4'
    iz = '2 2 1 7 1 2 6'
    subdomain_id = '
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 3 3 3 0 0
      0 0 3 3 3 0 0
      0 0 3 3 3 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 2 3 3 3 2 0
      0 2 1 1 1 2 0
      0 2 1 1 1 2 0
      0 2 2 2 2 2 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 3 3 3 0 0
      0 0 3 3 3 0 0
      0 0 3 3 3 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      '
  []
  [hollow_concrete]
    type = BlockDeletionGenerator
    input = bulk
    block = 1
    new_boundary = inner_cavity
  []
  [rename_blocks]
    type = RenameBlockGenerator
    input = hollow_concrete
    old_block = '0 2 3'
    new_block = 'concrete water Al'
  []

  [add_concrete_outer_boundary]
    type = RenameBoundaryGenerator
    input = rename_blocks
    old_boundary = 'left right front bottom top back'
    new_boundary = 'air_boundary air_boundary air_boundary air_boundary air_boundary ground'
  []
  [add_water_concrete_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_concrete_outer_boundary
    primary_block = water
    paired_block = concrete
    new_boundary = 'water_boundary'
  []
  [add_water_concrete_interface_inwards]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_water_concrete_interface
    primary_block = concrete
    paired_block = water
    new_boundary = 'water_boundary_inwards'
  []
[]
