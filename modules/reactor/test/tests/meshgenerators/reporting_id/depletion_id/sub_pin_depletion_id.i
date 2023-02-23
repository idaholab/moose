[Mesh]
  [pin1_base]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '4 4 4 4'
    background_intervals = 2
    polygon_size = 0.63
    polygon_size_style = 'apothem'
    ring_radii = '0.2 0.4 0.5'
    ring_intervals = '2 2 1'
    preserve_volumes = on
    flat_side_up = true
    sector_id_name = 'sector_id'
    ring_id_name = 'ring_id'
  []
  [pin1_a]
    type = SubdomainExtraElementIDGenerator
    input = pin1_base
    subdomains = '1 2 3 4 5'
    extra_element_id_names = 'material_id'
    extra_element_ids = '1 1 1 8 9'
  []
  [pin1]
    type = RenameBoundaryGenerator
    input = pin1_a
    old_boundary = '10002 15002 10004 15004 10001 15001 10003 15003'
    new_boundary = 'left left right right top top bottom bottom'
  []

  [pin2_base]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '4 4 4 4'
    background_intervals = 2
    polygon_size = 0.63
    polygon_size_style = 'apothem'
    ring_radii = '0.15 0.3 0.4'
    ring_intervals = '2 3 1'
    preserve_volumes = on
    flat_side_up = true
  []
  [pin2_a]
    type = SubdomainExtraElementIDGenerator
    input = pin2_base
    subdomains = '1 2 3 4 5'
    extra_element_id_names = 'material_id'
    extra_element_ids = '2 2 2 8 9'
  []
  [pin2]
    type = RenameBoundaryGenerator
    input = pin2_a
    old_boundary = '10002 15002 10004 15004 10001 15001 10003 15003'
    new_boundary = 'left left right right top top bottom bottom'
  []

  [assembly]
    type = CartesianIDPatternedMeshGenerator
    inputs = 'pin1 pin2'
    pattern = ' 1  0;
                0  1'
    assign_type = 'cell'
    id_name = 'pin_id'
  []

  [depletion_id]
    type = DepletionIDGenerator
    input = 'assembly'
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

[AuxVariables]
  [sector_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [ring_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [depletion_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_sector_id]
    type = ExtraElementIDAux
    variable = sector_id
    extra_id_name = sector_id
  []
  [set_ring_id]
    type = ExtraElementIDAux
    variable = ring_id
    extra_id_name = ring_id
  []
  [set_depletion_id]
    type = ExtraElementIDAux
    variable = depletion_id
    extra_id_name = depletion_id
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
