[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    ix = 3
    iy = 1
    iz = 5
    dx = 1
    dy = 2
    dz = 1.5
    dim = 3
  []
  [new_block]
    type = ParsedSubdomainMeshGenerator
    input = cmg
    combinatorial_geometry = 'x>0.6&z<1.1'
    block_name = 1
    block_id = 1
  []
[]

[Positions]
  [nodes]
    type = NodePositions
  []
  [nodes_block_1]
    type = NodePositions
    block = 1
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [num_pos]
    type = NumPositions
    positions = nodes
  []
  [num_pos_1]
    type = NumPositions
    positions = nodes_block_1
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]
