[Mesh]
  [subchannel]
  type = DetailedTriSubChannelMeshGenerator
  nrings = 9
  n_cells = 200
  flat_to_flat = 0.185
  heated_length = 2.0
  rod_diameter = 0.01
  pitch = 0.012
  spacer_z = '0 0.2 0.4 0.6 0.8'
  spacer_k = '0.1 0.1 0.1 0.1 0.10'
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

