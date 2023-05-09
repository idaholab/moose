[Mesh]
  [sq_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    background_intervals = 2
    ring_radii = 4.0
    ring_intervals = 2
    ring_block_ids = '10 15'
    ring_block_names = 'center_tri center'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
    flat_side_up = true
  []
  [pattern]
    type = PatternedCartesianMeshGenerator
    inputs = 'sq_1'
    pattern = '0 0 0;
               0 0 0;
               0 0 0'
    background_intervals = 2
    background_block_id = 25
    background_block_name = 'assem_block'
    square_size = 36
  []
  [pattern_mod]
    type = PatternedCartesianPeripheralModifier
    input = pattern
    new_num_sector = 7
    input_mesh_external_boundary = 10000
  []
  [pattern2]
    type = PatternedCartesianMeshGenerator
    inputs = 'pattern'
    pattern_boundary = none
    generate_core_metadata = true
    pattern = '0 0 0;
               0 0 0;
               0 0 0'
  []
  [peripheral_ring]
    type = PeripheralRingMeshGenerator
    input = pattern
    peripheral_ring_block_id = 200
    peripheral_layer_num = 2
    input_mesh_external_boundary = 10000
    peripheral_ring_radius = 30
  []
  [trim]
    type = CartesianMeshTrimmer
    input = pattern
  []
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = false
  []
[]
