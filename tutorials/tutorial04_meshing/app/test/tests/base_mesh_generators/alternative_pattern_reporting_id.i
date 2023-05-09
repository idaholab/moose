[Mesh]
  [pin1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    background_intervals = 1
    ring_radii ='0.4 0.5'
    ring_intervals = '1 1'
    polygon_size = 0.63
    flat_side_up = true
  []

  [pin2]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    background_intervals = 1
    ring_radii ='0.4 0.5'
    ring_intervals = '1 1'
    polygon_size = 0.63
    flat_side_up = true
  []

  [assembly1]
    type = PatternedCartesianMeshGenerator
    inputs = 'pin1 pin2'
    pattern = ' 1  0  0  1;
                0  0  1  0;
                0  1  0  0;
                1  0  0  1'
    assign_type = 'pattern'
    id_name = 'pin_id'
    pattern_boundary = 'none'
    square_size = 5.04
    generate_core_metadata = false
  []

  [assembly2]
    type = PatternedCartesianMeshGenerator
    inputs = 'pin1 pin2'
    pattern = ' 0  1  1  0;
                1  0  0  1;
                1  0  0  1;
                0  1  1  0'
    id_pattern = ' 0  0  1  1;
                   0  0  1  1;
                   2  2  3  3;
                   2  2  3  3'
    assign_type = 'manual'
    id_name = 'pin_id'
    pattern_boundary = 'none'
    square_size = 5.04
    generate_core_metadata = false
  []

  [core]
    type = PatternedCartesianMeshGenerator
    inputs = 'assembly1 assembly2'
    pattern = '0  1;
               1  0'
    assign_type = 'cell'
    id_name = 'assembly_id'
    pattern_boundary = 'none'
    generate_core_metadata = true
  []
[]
