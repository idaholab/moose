[Mesh]
  [pin1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '4 4 4 4'
    background_intervals = 2
    polygon_size = 0.63
    polygon_size_style ='apothem'
    ring_radii = '0.2 0.4 0.5'
    ring_intervals = '2 2 1'
    ring_block_ids = '1 2 3 4'
    background_block_ids = '5'
    preserve_volumes = on
    flat_side_up = true
    sector_id_name = 'sector_id'
    ring_id_name = 'ring_id'
  []

  [pin2]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '4 4 4 4'
    background_intervals = 2
    polygon_size = 0.63
    polygon_size_style ='apothem'
    ring_radii = '0.15 0.3 0.4'
    ring_intervals = '2 3 1'
    ring_block_ids = '6 7 8 9'
    background_block_ids = '10'
    preserve_volumes = on
    flat_side_up = true
    sector_id_name = 'sector_id'
    ring_id_name = 'ring_id'
  []

  [assembly]
    type = PatternedCartesianMeshGenerator
    inputs = 'pin1 pin2'
    pattern = ' 1  0;
                0  1'
    assign_type = 'cell'
    id_name = 'pin_id'
    pattern_boundary = 'none'
  []

  [assembly_mat_id]
    type = SubdomainExtraElementIDGenerator
    input = assembly
    subdomains = '1 2 3 4 5
                  6 7 8 9 10'
    extra_element_id_names = 'material_id'
    extra_element_ids = '1 1 1 8 9
                         2 2 2 8 9'
  []

  [depletion_id]
    type = DepletionIDGenerator
    input = 'assembly_mat_id'
    id_name = 'pin_id sector_id ring_id'
    material_id_name = 'material_id'
    exclude_id_name = 'material_id ring_id'
    exclude_id_value = '8 9; 0'
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
    extra_element_ids_to_output = 'sector_id ring_id depletion_id'
  []
[]
