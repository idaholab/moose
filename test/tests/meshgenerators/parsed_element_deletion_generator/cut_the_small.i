[Mesh]
  [elem1]
    type = ElementGenerator
    nodal_positions = '0 0 0
    1 0 0
    1 1 0'
    element_connectivity = '0 1 2'
    elem_type = "TRI3"
  []
  [elem2]
    type = ElementGenerator
    nodal_positions = '0 0 0
    1e-2 0 0
    1e-2 1e-2 0'
    element_connectivity = '0 1 2'
    elem_type = "TRI3"
  []
  [combine]
    type = CombinerGenerator
    inputs = 'elem1 elem2'
  []
  [delete]
    type = ParsedElementDeletionGenerator
    input = combine
    expression = 'volume < 0.1'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters]
  [mesh]
    type = MeshInfo
    outputs = json
    items = 'num_nodes'
  []
[]

[Outputs]
  [json]
    type = JSON
    execute_system_information_on = NONE
  []
[]
