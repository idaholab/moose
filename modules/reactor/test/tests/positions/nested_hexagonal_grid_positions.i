[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Positions]
  [hex_grid]
    type = HexagonalGridPositions
    center = '0 0 0'
    nr = 2
    # the lattice flat to flat is very large compared to the pitch * nr
    # the duct would be in between the lattice and the lattice
    lattice_flat_to_flat = 7
    pin_pitch = 0.5
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
  []
  [parent_grid]
    type = HexagonalGridPositions
    center = '0 0 0'
    nr = 2
    positions_pattern_indexing = 'hex_grid 1
                                  hex_grid_2rings_exclusions 2'
    positions_pattern = '1 1;
                        2 2 1;
                        -1 1'
    lattice_flat_to_flat = 12
    pin_pitch = 4
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

[Postprocessors]
  [n1]
    type = NumPositions
    positions = hex_grid
  []
  [n2]
    type = NumPositions
    positions = hex_grid_2rings_exclusions
  []
  [ntot]
    type = NumPositions
    positions = parent_grid
  []
[]
