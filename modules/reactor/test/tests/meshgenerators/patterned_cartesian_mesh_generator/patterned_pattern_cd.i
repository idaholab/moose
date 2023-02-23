[Mesh]
  [square_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    background_intervals = 1
    ring_radii = 3.0
    ring_intervals = 1
    ring_block_ids = '10'
    ring_block_names = 'center_tri_1'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
    flat_side_up = true
  []
  [pattern_1]
    type = PatternedCartesianMeshGenerator
    inputs = 'square_1'
    pattern = '0 0 0 0;
               0 0 0 0;
               0 0 0 0;
               0 0 0 0'
    square_size = 48
    background_intervals = 1
    rotate_angle = 0
    background_block_id = 1500
  []
  [cd_1]
    type = CartesianConcentricCircleAdaptiveBoundaryMeshGenerator
    num_sectors_per_side = '2 2 2 2'
    background_intervals = 2
    square_size = 48
    sides_to_adapt = '0 3'
    meshes_to_adapt_to = 'pattern_1 pattern_1'
    is_control_drum = true
  []
  [pattern_2]
    type = PatternedCartesianMeshGenerator
    inputs = 'pattern_1 cd_1'
    pattern = '0 0;
               1 0'
    pattern_boundary = none
    generate_core_metadata = true
    generate_control_drum_positions_file = true
    assign_control_drum_id = true
  []
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = false
  []
[]
