[Mesh]
  [subchannel]
    type = DetailedTriSubChannelMeshGenerator
    nrings = 3
    n_cells = 50
    flat_to_flat = 0.05319936
    heated_length = 0.87
    unheated_length_entry = 0.0
    unheated_length_exit = 0.402
    pin_diameter = 8.2e-3
    pitch = 0.01148
  []
[]

[AuxVariables]
  [mdot]
  []
  [SumWij]
  []
  [P]
  []
  [DP]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
  [mu]
  []
  [S]
  []
  [w_perim]
  []
  [q_prime]
  []
[]

[Problem]
  type = NoSolveProblem
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
[]
