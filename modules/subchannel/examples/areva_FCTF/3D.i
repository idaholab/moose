###################################################
# Geometric parameters
#####################################
fuel_pin_pitch = 0.01125 #m
fuel_pin_diameter = 0.0095 #m
inner_duct_in = 0.09164 #m
n_rings = 5
unheated_length_entry = 1.14 #m
heated_length = 1.71 #m
unheated_length_exit = 0.855 #m
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = DetailedTriSubChannelMeshGenerator
    nrings = '${n_rings}'
    n_cells = 65
    flat_to_flat = '${inner_duct_in}'
    unheated_length_entry = ${unheated_length_entry}
    heated_length = ${heated_length}
    unheated_length_exit = ${unheated_length_exit}
    rod_diameter = '${fuel_pin_diameter}'
    pitch = '${fuel_pin_pitch}'
  []

  [fuel_pins]
    type = DetailedTriPinMeshGenerator
    input = subchannel
    nrings = '${n_rings}'
    n_cells = 65
    unheated_length_entry = ${unheated_length_entry}
    heated_length = ${heated_length}
    unheated_length_exit = ${unheated_length_exit}
    rod_diameter = '${fuel_pin_diameter}'
    pitch = '${fuel_pin_pitch}'
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
  [displacement]
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
