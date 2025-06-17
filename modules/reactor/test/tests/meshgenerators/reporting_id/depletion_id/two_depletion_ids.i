[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    background_intervals = 1
    polygon_size = 0.63
    polygon_size_style ='apothem'
    ring_radii = '0.2 0.5'
    ring_intervals = '2 1'
    ring_block_ids = '1 2 3'
    background_block_ids = '4'
    preserve_volumes = on
    flat_side_up = true
  []

  [assembly]
    type = PatternedCartesianMeshGenerator
    inputs = pin
    pattern = ' 0  0;
                0  0'
    assign_type = 'cell'
    id_name = 'pin_id'
    pattern_boundary = 'none'
  []

  [core]
    type = PatternedCartesianMeshGenerator
    inputs = assembly
    pattern = ' 0 0;
                0 0'
    assign_type = 'cell'
    id_name = 'assembly_id'
    pattern_boundary = 'none'
    generate_core_metadata = true
  []

  [mat_id]
    type = SubdomainExtraElementIDGenerator
    input = core
    subdomains = '1 2 3 4'
    extra_element_id_names = 'material_id'
    extra_element_ids = '1 1 2 3'
  []

  [depl_map]
    type = DepletionIDGenerator
    input = mat_id
    id_name = 'assembly_id pin_id'
    material_id_name = 'material_id'
  []

  [assemblywise]
    type = DepletionIDGenerator
    input = 'depl_map'
    id_name = 'assembly_id'
    material_id_name = 'material_id'
    generated_id_name = another_depletion_id
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
    extra_element_ids_to_output = 'depletion_id another_depletion_id'
  []
[]
