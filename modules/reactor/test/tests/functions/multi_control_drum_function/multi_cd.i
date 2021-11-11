[Problem]
  solve = false
[]

[Mesh]
  [hex_unit]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6 # must be six to use hex pattern
    num_sectors_per_side= '2 2 2 2 2 2'
    background_intervals=1
    polygon_size = 1

    ring_radii = '0.9'
    ring_intervals = '1'
  []
  [patterned]
    type = PatternedHexMeshGenerator
    inputs = 'hex_unit'
    pattern_boundary = hexagon
    background_intervals = 1
    hexagon_size = 3.5

    pattern = '0 0;
              0 0 0;
               0 0'
  []
  [cd_1]
    type = HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'patterned'
    sides_to_adapt = '4'

    num_sectors_per_side= '2 2 2 2 2 2'
    hexagon_size = 3.5
    background_intervals= 1

    ring_radii = '2 3'
    ring_intervals = '1 1'

    block_id_shift = 5000
    is_control_drum = true
  []
  [cd_2]
    type = HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'patterned'
    sides_to_adapt = '5'

    num_sectors_per_side= '2 2 2 2 2 2'
    hexagon_size = 3.5
    background_intervals= 1

    ring_radii = '2 3'
    ring_intervals = '1 1'

    block_id_shift = 5000
    is_control_drum = true
  []
  [cd_3]
    type = HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'patterned'
    sides_to_adapt = '0'

    num_sectors_per_side= '2 2 2 2 2 2'
    hexagon_size = 3.5
    background_intervals= 1

    ring_radii = '2 3'
    ring_intervals = '1 1'

    block_id_shift = 5000
    is_control_drum = true
  []
  [cd_4]
    type = HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'patterned'
    sides_to_adapt = '1'

    num_sectors_per_side= '2 2 2 2 2 2'
    hexagon_size = 3.5
    background_intervals= 1

    ring_radii = '2 3'
    ring_intervals = '1 1'

    block_id_shift = 5000
    is_control_drum = true
  []
  [cd_5]
    type = HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'patterned'
    sides_to_adapt = '2'

    num_sectors_per_side= '2 2 2 2 2 2'
    hexagon_size = 3.5
    background_intervals= 1

    ring_radii = '2 3'
    ring_intervals = '1 1'

    block_id_shift = 5000
    is_control_drum = true
  []
  [cd_6]
    type = HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'patterned'
    sides_to_adapt = '3'

    num_sectors_per_side= '2 2 2 2 2 2'
    hexagon_size = 3.5
    background_intervals= 1

    ring_radii = '2 3'
    ring_intervals = '1 1'

    block_id_shift = 5000
    is_control_drum = true
  []
  [core]
    type = PatternedHexMeshGenerator
    inputs = 'patterned cd_1 cd_2 cd_3 cd_4 cd_5 cd_6'
    #             0       1    2    3    4    5    6
    pattern_boundary = none
    generate_core_metadata = true
    generate_control_drum_positions_file = true
    pattern = '1 6;
              2 0 5;
               3 4'
  []
[]

[AuxVariables]
  [cd_param]
    family = MONOMIAL
    order = CONSTANT
    block = 5002
  []
  [cd_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cd_param_assign]
    type = FunctionAux
    variable = cd_param
    function = cd_param_func
    execute_on = 'initial linear timestep_end'
  []
  [set_cd_id]
    type = ExtraElementIDAux
    variable = cd_id
    extra_id_name = control_drum_id
    execute_on = 'initial timestep_end'
  []
[]

[Functions]
  [cd_param_func]
    type = MultiControlDrumFunction
    mesh_generator = core
    angular_speeds = '2 4 8 16 64 128'
    start_angles = '0 0 0 0 0 0'
    angle_ranges = '90 90 90 90 90 90'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 5
[]

[Outputs]
  [default]
    type = Exodus
  []
[]
