[Mesh]
  [dir1]
    type = ElementGenerator
    nodal_positions = '0 0 0
                     1 0 0
                     0 1 0'
    element_connectivity = '0 1 2'
    elem_type = 'TRI3'
  []
  [dir2]
    type = ElementGenerator
    nodal_positions = '1 1 0
                     1 0 0
                     0 1 0'
    element_connectivity = '0 1 2'
    elem_type = 'TRI3'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'dir1 dir2'
  []
  [diag]
    type = MeshRepairGenerator
    input = combine
    fix_node_overlap = true
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
