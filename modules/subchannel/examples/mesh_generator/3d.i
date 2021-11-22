[Mesh]
  type = DetailedQuadSubChannelMesh
  nx = 6
  ny = 6
  n_cells = 20
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095
  heated_length = 1.00
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
