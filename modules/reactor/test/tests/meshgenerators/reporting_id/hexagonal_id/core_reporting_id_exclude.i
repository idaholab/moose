[Mesh]
  [assembly]
    type = SimpleHexagonGenerator
    hexagon_size = 5.0
    hexagon_size_style = 'apothem'
    block_id = '1'
  []
  [dummy]
    type = SimpleHexagonGenerator
    hexagon_size = 5.0
    hexagon_size_style = 'apothem'
    block_id = '2'
  []

  [core]
    type = PatternedHexMeshGenerator
    inputs = 'assembly dummy'
    pattern_boundary = none
    pattern = '  1 0 1;
                0 0 0 0;
               1 0 0 0 1;
                0 0 0 0;
                 1 0 1'
    assign_type = 'cell'
    id_name = 'assembly_id'
    exclude_id = 'dummy'
  []

  [del_dummy]
    type = BlockDeletionGenerator
    block = 2
    input = core
    new_boundary = core_out
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [assembly_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_assembly_id]
    type = ExtraElementIDAux
    variable = assembly_id
    extra_id_name = assembly_id
  []
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = timestep_end
    output_extra_element_ids = true
    extra_element_ids_to_output = 'assembly_id'
  []
[]
