[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    ix = 2
    iy = 1
    iz = 1
    dx = 1
    dy = 2
    dz = 1.5
    dim = 3
  []
  [new_block]
    type = ParsedSubdomainMeshGenerator
    input = cmg
    combinatorial_geometry = 'x>0.6'
    block_name = 1
    block_id = 1
  []
[]

[Positions]
  [all_mesh]
    type = QuadraturePointsPositions
    # For testing reproducibility
    auto_sort = true
  []
  [block_1]
    type = QuadraturePointsPositions
    block = 1
    # For testing reproducibility
    auto_sort = true
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
    positions = all_mesh
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]
