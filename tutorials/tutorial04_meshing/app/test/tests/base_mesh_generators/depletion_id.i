[Mesh]
  [pin1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '4 4 4 4'
    background_intervals = 1
    ring_radii ='0.4 0.5'
    ring_intervals = '4 1'
    ring_block_ids = '1 2 5'
    background_block_ids = '6'
    polygon_size = 0.63
    flat_side_up = true
  []
  [pin2]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '4 4 4 4'
    background_intervals = 1
    ring_radii ='0.4 0.5'
    ring_intervals = '4 1'
    ring_block_ids = '3 4 5'
    background_block_ids = '6'
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
    assign_type = 'cell'
    id_name = 'pin_id'
    pattern_boundary = 'none'
    square_size = 5.04
    generate_core_metadata = false
  []
  [core]
    type = PatternedCartesianMeshGenerator
    inputs = 'assembly1'
    pattern = '0 0;
               0 0'
    assign_type = 'cell'
    id_name = 'assembly_id'
    pattern_boundary = 'none'
    generate_core_metadata = true
  []
  [core_mat_id]
    type = SubdomainExtraElementIDGenerator
    input = core
    subdomains = '1 2 3 4 5 6'
    extra_element_id_names = 'material_id'
    extra_element_ids = '1 1 2 2 3 4'
  []
  [depletion_id]
    type = DepletionIDGenerator
    input = 'core_mat_id'
    id_name = 'assembly_id pin_id'
    material_id_name = 'material_id'
    exclude_id_name = 'material_id'
    exclude_id_value = '3 4'
  []
[]
