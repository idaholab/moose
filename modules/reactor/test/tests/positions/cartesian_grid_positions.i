[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Positions]
  # active = 'cart_grid'
  [cart_grid]
    type = CartesianGridPositions
    center = '0 0 0'
    nx = 3
    ny = 2
    nz = 1
    dx = 10
    dy = 4
    dz = 2
    outputs = 'out'
  []
  [cart_grid_exclusions_2D]
    type = CartesianGridPositions
    center = '100 100 0'
    nx = 3
    ny = 2
    nz = 1
    dx = 10
    dy = 4
    dz = 2
    pattern = '1 1 1;
               2 1 1'
    include_in_pattern = '1'
    outputs = 'out'
  []
  [cart_grid_exclusions_3D]
    type = CartesianGridPositions
    center = '100 100 0'
    nx = 3
    ny = 2
    nz = 2
    dx = 10
    dy = 4
    dz = 2
    pattern = '1 1 1;
               2 1 1|
               10 1 0;
               1 1 0'
    include_in_pattern = '1'
    outputs = 'out'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]
