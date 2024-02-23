[Mesh]
  [pin1]
    type = PolygonConcentricCircleMeshGenerator
    preserve_volumes = true
    ring_radii = 0.4
    ring_intervals = 1
    background_intervals = 1
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    polygon_size = 0.5
  []
  [pin2]
    type = PolygonConcentricCircleMeshGenerator
    preserve_volumes = true
    ring_radii = 0.4
    ring_intervals = 1
    background_intervals = 1
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    polygon_size = 0.5
  []
  [assembly]
    type = PatternedHexMeshGenerator
    inputs = 'pin1 pin2'
    pattern_boundary = hexagon
    pattern = '  1 0 1;
                0 0 0 0;
               1 0 1 0 1;
                0 0 0 0;
                 1 0 1'
    hexagon_size = 2.6
    duct_sizes = '2.4 2.5'
    duct_intervals = '1 1'
    assign_type = 'cell pattern manual manual'
    id_name = 'pin_id pin_type_id manual_1_id manual_2_id'
    id_pattern = '2 2 2;
                 2 1 1 2;
                2 1 0 1 2;
                 2 1 1 2;
                  2 2 2|
                  1 1 1;
                 2 2 2 2;
                3 3 3 3 3;
                 4 4 4 4;
                  5 5 5'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = timestep_end
    output_extra_element_ids = true
    extra_element_ids_to_output = 'pin_id pin_type_id manual_1_id manual_2_id'
  []
[]
