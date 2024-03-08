[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Positions]
  active = 'hex_grid'
  [hex_grid]
    type = HexagonalGridPositions
    center = '0 0 0'
    nr = 2
    # the lattice flat to flat is very large compared to the pitch * nr
    # the duct would be in between the lattice and the lattice
    lattice_flat_to_flat = 7
    pin_pitch = 0.5
    outputs = 'out'
  []
  [hex_grid_2rings_exclusions]
    type = HexagonalGridPositions
    center = '0 0 0'
    nr = 2
    pattern = '1 1;
             2  1  2;
               1 1'
    include_in_pattern = '1'
    lattice_flat_to_flat = 4
    pin_pitch = 0.5
    outputs = 'out'
  []
  [hex_grid_3rings_exclusions]
    type = HexagonalGridPositions
    center = '0 0 0'
    nr = 3
    pattern = '1 1 1;
              1 1 1 1;
             2 1 3 2 3;
              1 1 3 1;
               1 1 1'
    include_in_pattern = '1 2'
    lattice_flat_to_flat = 4
    pin_pitch = 0.5
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
