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

  [assembly]
    type = PatternedCartesianMeshGenerator
    inputs = 'pin1 pin2'
    pattern = ' 1  0  1  0;
                0  1  0  1;
                1  0  1  0;
                0  1  0  1'
    assign_type = 'cell pattern manual manual'
    id_name = 'pin_id pin_type_id manual_1_id manual_2_id'
    id_pattern='0 0 1 1;
                0 0 1 1;
                2 2 3 3;
                2 2 3 3|
                1 1 1 1;
                2 2 2 2;
                3 3 3 3;
                4 4 4 4'
    pattern_boundary = 'none'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [pin_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_pin_id]
    type = ExtraElementIDAux
    variable = pin_id
    extra_id_name = pin_id
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
