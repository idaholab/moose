[Mesh]
  [hex_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    ring_radii = 4.0
    ring_intervals = 1
    ring_block_ids = '10'
    ring_block_names = 'center'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
  []
  [pattern]
    type = PatternedHexMeshGenerator
    inputs = 'hex_1'
    pattern = '0 0;
              0 0 0;
               0 0'
    background_intervals = 2
    background_block_id = 25
    background_block_name = 'assem_block'
    hexagon_size = 18
  []
  [trim_0]
    type = HexagonMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 1 1 1 1 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_1]
    type = HexagonMeshTrimmer
    input = pattern
    trim_peripheral_region = '0 1 1 1 0 0'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_2]
    type = HexagonMeshTrimmer
    input = pattern
    trim_peripheral_region = '0 0 1 1 1 0'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_3]
    type = HexagonMeshTrimmer
    input = pattern
    trim_peripheral_region = '0 0 0 1 1 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_4]
    type = HexagonMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 0 0 0 1 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_5]
    type = HexagonMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 1 0 0 0 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [trim_6]
    type = HexagonMeshTrimmer
    input = pattern
    trim_peripheral_region = '1 1 1 0 0 0'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [pattern2]
    type = PatternedHexMeshGenerator
    inputs = 'trim_0 trim_1 trim_2 trim_3 trim_4 trim_5 trim_6'
    pattern_boundary = none
    generate_core_metadata = true
    pattern = '3 2;
              4 0 1;
               5 6'
  []
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = false
  []
[]
