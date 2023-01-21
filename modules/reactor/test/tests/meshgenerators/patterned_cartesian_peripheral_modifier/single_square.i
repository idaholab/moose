[Mesh]
  inactive = 'pmg2 pmg3'
  [square]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    flat_side_up = true
    background_intervals = 1
    ring_radii = 4.0
    ring_intervals = 2
    ring_block_ids = '10 15'
    ring_block_names = 'center_tri center'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
  []
  [pattern]
    type = PatternedCartesianMeshGenerator
    inputs = 'square'
    pattern = '0 0 0;
               0 0 0;
               0 0 0'
    background_intervals = 2
    square_size = 34
    #duct_sizes = '15 15.5'
    #duct_intervals = '1 2'
  []
  [pmg]
    type = PatternedCartesianPeripheralModifier
    input = pattern
    input_mesh_external_boundary = 10000
    new_num_sector = 10
    num_layers = 2
  []
  [pmg2]
    type = PatternedCartesianPeripheralModifier
    input = pmg
    input_mesh_external_boundary = 10000
    new_num_sector = 10
    num_layers = 2
  []
  [pmg3]
    type = PatternedHexPeripheralModifier
    input = pattern
    input_mesh_external_boundary = 10000
    new_num_sector = 10
    num_layers = 2
  []
[]
