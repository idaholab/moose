[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [delete]
    type = ParsedElementDeletionGenerator
    input = gmg
    expression = 'y < 0.49'
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
    items = 'num_nodes num_elements'
    outputs = json
  []
[]

[Outputs]
  [json]
    type = JSON
    execute_system_information_on = NONE
    execute_on = 'TIMESTEP_END'
  []
[]
