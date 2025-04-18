[GlobalParams]
  ######## Geometry #
  nx = 7
  ny = 3
  n_cells = 48
  pitch = 0.014605
  pin_diameter = 0.012065
  gap = 0.0015875
  heated_length = 1.2192
[]

[Mesh]
  [subchannel]
    type = SCMDetailedQuadSubChannelMeshGenerator
  []
  [pins]
    type = SCMDetailedQuadPinMeshGenerator
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
