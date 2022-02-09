[Mesh]
  [subchannel]
    type = DetailedQuadSubChannelMeshGenerator
    nx = 7
    ny = 3
    n_cells = 48
    pitch = 0.014605
    rod_diameter = 0.012065
    gap = 0.0015875
    heated_length = 1.2192
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
