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
    pattern = ' 1  0  1  0;
                0  1  0  1;
                1  0  1  0;
                0  1  0  1'
    assign_type = 'cell'
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
    assign_type = 'cell'
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
  [assembly_id]
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
  [set_assembly_id]
    type = ExtraElementIDAux
    variable = assembly_id
    extra_id_name = assembly_id
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
