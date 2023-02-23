[Mesh]
  [sq_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    background_intervals = 1
    ring_radii = 4.0
    ring_intervals = 1
    ring_block_ids = '10'
    ring_block_names = 'center'
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
  [trim_0]
    type = CartesianMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 1 1 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_1]
    type = CartesianMeshTrimmer
    input = pattern
    trim_peripheral_region = '0 0 1 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_2]
    type = CartesianMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 0 1 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_3]
    type = CartesianMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 0 0 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_4]
    type = CartesianMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 1 0 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_5]
    type = CartesianMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 1 0 0'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_6]
    type = CartesianMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 1 1 0'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_7]
    type = CartesianMeshTrimmer
    input = pattern
    trim_peripheral_region = '0 1 1 0'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_8]
    type = CartesianMeshTrimmer
    input = pattern
    trim_peripheral_region = '0 1 1 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [pattern2]
    type = PatternedCartesianMeshGenerator
    inputs = 'trim_0 trim_1 trim_2 trim_3 trim_4 trim_5 trim_6 trim_7 trim_8'
    pattern_boundary = none
    generate_core_metadata = true
    pattern = '3 2 1;
               4 0 8;
               5 6 7'
  []
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = false
  []
[]
