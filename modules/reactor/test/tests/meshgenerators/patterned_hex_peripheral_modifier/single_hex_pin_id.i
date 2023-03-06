[Mesh]
  [hex]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    ring_radii = 4.0
    ring_intervals = 2
    ring_block_ids = '10 15'
    ring_block_names = 'center_tri center'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
  []
  [pattern]
    type = PatternedHexMeshGenerator
    inputs = 'hex'
    pattern = '0 0;
              0 0 0;
               0 0'
    background_intervals = 2
    hexagon_size = 17
    assign_type = 'cell'
    id_name = 'pin_id'
  []
  [pmg]
    type = PatternedHexPeripheralModifier
    input = pattern
    input_mesh_external_boundary = 10000
    new_num_sector = 10
    num_layers = 2
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
  [exodus_1]
    type = Exodus
    enable = false
    execute_on = TIMESTEP_END
  []
  [exodus_2]
    type = Exodus
    enable = false
    execute_on = TIMESTEP_END
  []
[]
