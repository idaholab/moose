[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    preserve_volumes = true
    ring_radii = 0.4
    ring_intervals = 2
    background_intervals = 1
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    polygon_size = 0.5
  []
  [assembly]
    type = PatternedHexMeshGenerator
    inputs = 'pin'
    pattern_boundary = hexagon
    pattern = '  0 0 0;
                0 0 0 0;
               0 0 0 0 0;
                0 0 0 0;
                 0 0 0'
    hexagon_size = 2.6
    duct_sizes = '2.4 2.5'
    duct_intervals = '1 1'
    assign_type = 'cell'
    id_name = 'pin_id'
  []
  [core]
    type = PatternedHexMeshGenerator
    inputs = 'assembly'
    pattern_boundary = none
    pattern = ' 0 0;
               0 0 0;
                0 0'
    assign_type = 'cell'
    id_name = 'assembly_id'
     generate_core_metadata = true
  []
[]

[AuxVariables]
  [variable_1]
    family = LAGRANGE
    order = FIRST
  []
  [variable_2]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[AuxKernels]
  [set_variable1]
    type = ParsedAux
    use_xyzt = true
    expression = 'exp(-0.05*x*x)*exp(-0.05*y*y)'
    variable = variable_1
  []
  [set_variable2]
    type = ParsedAux
    use_xyzt = true
    expression = 'cos(0.1*x) * cos(0.1*y)'
    variable = variable_2
    block = '1 2'
  []
[]

[VectorPostprocessors]
  [integral]
     type = ExtraIDIntegralVectorPostprocessor
     variable = 'variable_1 variable_2'
     id_name = 'assembly_id pin_id'
  []
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = timestep_end
[]
