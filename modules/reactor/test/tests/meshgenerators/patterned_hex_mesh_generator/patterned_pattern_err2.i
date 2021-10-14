[Mesh]
  [hex_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 2
    ring_radii = 4.0
    ring_intervals = 2
    ring_block_ids = '10 15'
    ring_block_names = 'center_tri center'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
  []
  [pattern_1]
    type = PatternedHexMeshGenerator
    inputs = 'hex_1'
    pattern = '0 0;
              0 0 0;
               0 0'
    hexagon_size = 15
    background_block_id = 80
    background_block_name = hex_background
  []
  [pattern_2]
    type = PatternedHexMeshGenerator
    inputs = 'hex_1'
    pattern = '0 0;
              0 0 0;
               0 0'
    hexagon_size = 16
    background_block_id = 80
    background_block_name = hex_background
  []
  [pattern_3]
    type = PatternedHexMeshGenerator
    inputs = 'pattern_1 pattern_2'
    pattern_boundary = none
    generate_core_metadata = true
    pattern = '0 0;
              0 1 0;
               0 0'
  []
[]
