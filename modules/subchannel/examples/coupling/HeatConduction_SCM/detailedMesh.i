[GlobalParams]
  nx = 2
  ny = 2
  n_cells = 10
  pitch = 0.014605
  pin_diameter = 0.012065
  gap = 0.0015875
  heated_length = 1.0
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
  [Dpin]
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
