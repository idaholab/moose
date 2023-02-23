[Mesh]
  [pin_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    ring_radii = 2.0
    ring_intervals = 1
    ring_block_ids = '50'
    ring_block_names = center_1
    background_block_ids = 40
    background_block_names = background_1
    polygon_size = 3.0
    preserve_volumes = on
    quad_center_elements = true
  []
  [assm1]
    type = PatternedHexMeshGenerator
    inputs = 'pin_1'
    id_name ='test_id1'
    hexagon_size = 20
    assign_type = pattern
    uniform_mesh_on_sides = true
    background_intervals = 1
    background_block_id = 40
    background_block_name = background_1
    pattern = '0 0 0 0;
              0 0 0 0 0;
             0 0 0 0 0 0;
            0 0 0 0 0 0 0;
             0 0 0 0 0 0;
              0 0 0 0 0;
               0 0 0 0'
  []
  [assm_up]
    type = TriPinHexAssemblyGenerator
    ring_radii = '7 8;7 8;7 8'
    ring_intervals = '1 1;1 1;1 1'
    ring_block_ids = '200 400;200 400;200 400'
    background_block_ids = '40'
    num_sectors_per_side = 14
    background_intervals = 2
    hexagon_size = ${fparse 40.0/sqrt(3.0)}
    ring_offset = 0.6
    azimuthal_interval_style = equal_length
    assembly_orientation = pin_up
  []
  [pattern]
    type = PatternedHexMeshGenerator
    inputs = 'assm1 assm_up'
    id_name ='test_id2'
    pattern = '0 0;
              1 1 1;
               0 0'
    pattern_boundary = none
    generate_core_metadata = true
  []
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = false
  []
[]
