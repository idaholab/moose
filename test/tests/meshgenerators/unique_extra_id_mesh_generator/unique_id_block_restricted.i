[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = unique_id.e
    exodus_extra_element_integers = 'id1 id0'
  []
  [assign_block_id]
    type = SubdomainBoundingBoxGenerator
    input = fmg
    bottom_left = '0 -2 0'
    top_right = '2 0 0'
    block_id = 1
  []
  [parse_id]
    type = UniqueExtraIDMeshGenerator
    input = assign_block_id
    id_name = 'id1 id0'
    new_id_name = 'parsed_id'
    restricted_subdomains = 1
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
