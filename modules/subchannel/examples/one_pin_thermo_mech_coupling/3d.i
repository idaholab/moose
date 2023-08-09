[GlobalParams]
  nx = 2
  ny = 2
  n_cells = 25
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095
  heated_length = 1.0
[]

[Mesh]
  [subchannel]
    type = DetailedQuadSubChannelMeshGenerator
  []
  [fuel_pins]
    type = DetailedQuadPinMeshGenerator
    input = subchannel
  []
[]

[AuxVariables]
  [mdot]
    block = subchannel
  []
  [SumWij]
    block = subchannel
  []
  [P]
    block = subchannel
  []
  [DP]
    block = subchannel
  []
  [h]
    block = subchannel
  []
  [T]
    block = subchannel
  []
  [rho]
    block = subchannel
  []
  [mu]
    block = subchannel
  []
  [S]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [q_prime]
    block = fuel_pins
  []
  [Tpin]
    block = fuel_pins
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
