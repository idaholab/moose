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
    nodal_positions = '0 0 0
                     1 0 0
                     1 1 0
                     0 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = 'QUAD4'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'dir1 dir2'
  []
  [separate]
    type = MeshRepairGenerator
    input = 'combine'
    separate_blocks_by_element_types = true
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
    items = 'subdomains'
  []
[]

[Outputs]
  [json]
    type = JSON
    execute_system_information_on = NONE
  []
[]
