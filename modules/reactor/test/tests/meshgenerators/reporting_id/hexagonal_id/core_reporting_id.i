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
  [assembly1]
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
    id_name = 'pin_id'
    assign_type = 'cell'
  []
  [assembly2]
    type = PatternedHexMeshGenerator
    inputs = 'pin1 pin2'
    pattern_boundary = hexagon
    pattern = '  0 0 0;
                0 1 1 0;
               0 1 0 1 0;
                0 1 1 0;
                 0 0 0'
    hexagon_size = 2.6
    duct_sizes = '2.4 2.5'
    duct_intervals = '1 1'
    id_name = 'pin_id'
    assign_type = 'cell'
  []
  [core]
    type = PatternedHexMeshGenerator
    inputs = 'assembly1 assembly2'
    pattern_boundary = none
    pattern = '1 1;
              1 0 1;
               1 1'
    generate_core_metadata = true
    id_name = 'assembly_id'
    assign_type = 'cell'
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
    extra_element_ids_to_output = 'pin_id assembly_id'
  []
[]
