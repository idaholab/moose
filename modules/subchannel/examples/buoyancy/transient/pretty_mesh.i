
[Mesh]
  type = DetailedQuadSubChannelMesh
  nx = 7
  ny = 3
  n_cells = 48
  n_blocks = 1
  pitch = 0.014605
  rod_diameter = 0.012065
  gap = 0.0015875
  heated_length = 1.2192
  spacer_z = '0.0 '
  spacer_k = '0.0 '
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
  type = Transient
[]
