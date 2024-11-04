pin_diameter = 5.84e-3
heated_length = 1.0
[Mesh]
  [subchannel]
    type = DetailedTriSubChannelMeshGenerator
    nrings = 2
    n_cells = 40
    flat_to_flat = 2.1e-2 # 3.41e-2
    heated_length = ${heated_length} # 0.5334
    pin_diameter = ${pin_diameter}
    pitch = 7.26e-3
  []

  [fuel_pins]
    type = DetailedTriPinMeshGenerator
    input = subchannel
    nrings = 2
    n_cells = 40
    heated_length = ${heated_length} # 0.5334
    pin_diameter = ${pin_diameter}
    pitch = 7.26e-3
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
