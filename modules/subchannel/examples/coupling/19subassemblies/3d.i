###################################################
# Geometric parameters
###################################################
#f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
scale_factor = 0.01
fuel_element_pitch = '${fparse 16.2471*scale_factor}'
inter_assembly_gap = '${fparse 0.4348*scale_factor}'
duct_thickness = '${fparse 0.3966*scale_factor}'
fuel_pin_pitch = '${fparse 0.8966*scale_factor}'
fuel_pin_diameter = '${fparse 0.7714*scale_factor}'
length_entry_fuel = '${fparse 160.92*scale_factor}'
length_heated_fuel = '${fparse 85.82*scale_factor}'
length_outlet_fuel = '${fparse 233.46*scale_factor}'
duct_outside = ${fparse fuel_element_pitch - inter_assembly_gap}
duct_inside = ${fparse duct_outside -  2 * duct_thickness}
n_rings = 10

[Mesh]
  [subchannel]
    type = SCMDetailedTriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = 50 #100
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pin_diameter = '${fparse fuel_pin_diameter}'
    pitch = '${fparse fuel_pin_pitch}'
  []

  [fuel_pins]
    type = SCMDetailedTriPinMeshGenerator
    input = subchannel
    nrings = '${fparse n_rings}'
    n_cells = 50
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
    pin_diameter = '${fparse fuel_pin_diameter}'
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
