[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = unique_id_cut_in.e
    exodus_extra_element_integers = 'id1 id0'
  []
  [parse_id]
    type = UniqueExtraIDMeshGenerator
    input = fmg
    id_name = 'id1 id0'
    new_id_name = 'parsed_id'
  []
  [deleter]
      type = PlaneDeletionGenerator
      point = '0 0 0'
      normal = '1 1 0'
      input = parse_id
      new_boundary = new_side
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [parsed_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [aux_parsed_id]
    type = ExtraElementIDAux
    variable = parsed_id
    extra_id_name = parsed_id
  []
[]


[Outputs]
  exodus = true
  execute_on = timestep_end
[]
