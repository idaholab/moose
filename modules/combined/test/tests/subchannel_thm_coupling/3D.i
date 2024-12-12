###################################################
# Geometric parameters
###################################################
n_cells = 25
n_rings = 3
fuel_pin_pitch = 7.26e-3
fuel_pin_diameter =  5.84e-3
inner_duct_in = 3.41e-2
heated_length = 1.0
###################################################
[TriSubChannelMesh]
  [subchannel]
    type = DetailedTriSubChannelMeshGenerator
    nrings = ${n_rings}
    n_cells = ${n_cells}
    flat_to_flat = ${inner_duct_in}
    heated_length = ${heated_length}
    pin_diameter = ${fuel_pin_diameter}
    pitch = ${fuel_pin_pitch}
  []

  [fuel_pins]
    type = DetailedTriPinMeshGenerator
    input = subchannel
    nrings = ${n_rings}
    n_cells = ${n_cells}
    pin_diameter = ${fuel_pin_diameter}
    heated_length = ${heated_length}
    pitch = ${fuel_pin_pitch}
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
  [Sij]
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
