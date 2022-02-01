[GlobalParams]
  nx = 5
  ny = 5
  n_cells = 36
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095
  heated_length = 3.6
[]

[Mesh]
  [subchannel]
    type = DetailedQuadSubChannelMeshGenerator
  []
  [pins]
    type = DetailedQuadPinMeshGenerator
    input = subchannel
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
  [Tpin]
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
