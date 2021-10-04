[Mesh]
  type = DetailedQuadSubChannelMesh
  nx = 3
  ny = 3
  n_cells = 100
  n_blocks = 1
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095
  unheated_length_entry = 2.5
  heated_length = 5.0
  unheated_length_exit = 2.5
  spacer_z = '0.0'
  spacer_k = '0.0'
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
