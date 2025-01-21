[Mesh]
  [subchannel]
    type = DetailedQuadSubChannelMeshGenerator
    nx = 6
    ny = 6
    n_cells = 50
    pitch = 0.0126
    pin_diameter = 0.00950
    gap = 0.00095
    heated_length = 3.658
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
  nl_rel_tol = 0.9
  l_tol = 0.9
[]
