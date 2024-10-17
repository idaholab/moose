[Mesh]
  [subchannel]
    type = DetailedQuadSubChannelMeshGenerator
    nx = 6
    ny = 6
    n_cells = 20
    pitch = 0.0126
    pin_diameter = 0.00950
    gap = 0.00095
    heated_length = 1.0
  []

  [fuel_pins]
    type = DetailedQuadPinMeshGenerator
    input = subchannel
    nx = 6
    ny = 6
    n_cells = 20
    pitch = 0.0126
    pin_diameter = 0.00950
    heated_length = 1.0
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
  [S]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [mu]
    block = subchannel
  []
  [q_prime]
    block = fuel_pins
  []
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
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
