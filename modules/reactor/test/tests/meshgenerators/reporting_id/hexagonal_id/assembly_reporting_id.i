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
    assign_type = 'cell'
    id_name = 'pin_id'
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
