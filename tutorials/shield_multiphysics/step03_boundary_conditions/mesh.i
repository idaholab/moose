[Mesh]
  [bulk]
    type = GeneratedMeshGenerator # Can generate simple lines, rectangles and rectangular prisms
    dim = 3
    nx = 25
    ny = 33
    nz = 20
    xmax = 10 # m
    ymax = 13 # m
    zmax = 8 # m
  []

  [create_inner_water]
    type = ParsedSubdomainMeshGenerator
    input = bulk
    # Create each water wall
    combinatorial_geometry = '(x > 2 & x < 3 & y > 2 & y < 11 & z > 1 & z < 5) |
                  (x > 6.5 & x < 7.5 & y > 2 & y < 11 & z > 1 & z < 5) |
                  (x > 2 & x < 7.5 & y > 2 & y < 3 & z > 1 & z < 5) |
                  (x > 2 & x < 7.5 & y > 10 & y < 11 & z > 1 & z < 5)'
    block_id = 2
  []

  [hollow_concrete]
    type = ParsedSubdomainMeshGenerator
    input = create_inner_water
    block_id = 1
    combinatorial_geometry = 'x > 3.5 & x < 6 & y > 3.5 & y < 9.5 & z > 1 & z < 5'
  []

  [rename_blocks]
    type = RenameBlockGenerator
    input = hollow_concrete
    old_block = '0 1 2'
    new_block = 'concrete cavity water'
  []

  [add_water_concrete_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = rename_blocks
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

  [add_inner_cavity]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_water_concrete_interface_inwards
    primary_block = concrete
    paired_block = cavity
    new_boundary = 'inner_cavity'
  []

  [add_concrete_outer_boundary]
    type = RenameBoundaryGenerator
    input = add_inner_cavity
    old_boundary = 'left right front bottom top back'
    new_boundary = 'air_boundary air_boundary air_boundary air_boundary air_boundary ground'
  []

  [remove_cavity]
    type = BlockDeletionGenerator
    input = 'add_concrete_outer_boundary'
    block = cavity
  []

  # [check_mesh]
  #   type = MeshDiagnosticsGenerator
  #   input = add_concrete_outer_boundary
  #   examine_sidesets_orientation = WARNING
  # []
[]
