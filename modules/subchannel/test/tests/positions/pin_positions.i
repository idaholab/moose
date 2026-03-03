###################################################
# Thermal-hydraulics parameters
###################################################
P_out = 758423 # Pa

###################################################
# Geometric parameters
###################################################

n_cells = 50

# units are cm - do not forget to convert to meter
scale_factor = 0.01
fuel_element_pitch = '${fparse 14.598*scale_factor}'
inter_assembly_gap = '${fparse 0.4*scale_factor}'
duct_thickness = '${fparse 0.3*scale_factor}'
fuel_pin_pitch = '${fparse 0.904*scale_factor}'
fuel_pin_diameter = '${fparse 0.8*scale_factor}'
wire_z_spacing = '${fparse 20.32*scale_factor}'
wire_diameter = '${fparse 0.103*scale_factor}'
n_rings = 9
# Reduced height for convenience
length_entry_fuel = '${fparse 20*scale_factor}'
length_heated_fuel = '${fparse 40*scale_factor}'
length_outlet_fuel = '${fparse 20*scale_factor}'
# height = '${fparse length_entry_fuel+length_heated_fuel+length_outlet_fuel}'
orifice_plate_height = '${fparse 5*scale_factor}'
duct_outside = '${fparse fuel_element_pitch - inter_assembly_gap}'
duct_inside = '${fparse duct_outside - 2 * duct_thickness}'

###################################################

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = ${n_cells}
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pin_diameter = '${fparse fuel_pin_diameter}'
    pitch = '${fparse fuel_pin_pitch}'
    dwire = '${fparse wire_diameter}'
    hwire = '${fparse wire_z_spacing}'
    spacer_z = '${fparse orifice_plate_height} ${fparse length_entry_fuel}'
    spacer_k = '0.5 0.5'
  []

  [fuel_pins]
    type = SCMTriPinMeshGenerator
    input = subchannel
    nrings = '${fparse n_rings}'
    n_cells = ${n_cells}
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
  []

  [duct]
    type = SCMTriDuctMeshGenerator
    input = fuel_pins
    nrings = '${fparse n_rings}'
    n_cells = ${n_cells}
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
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
    initial_condition = 0
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
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
    block = fuel_pins
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
  [displacement]
    block = subchannel
  []
  [ff]
    block = subchannel
  []
  [q_prime]
    block = fuel_pins
  []
  [mu]
    block = subchannel
  []
  [duct_heat_flux]
    block = duct
    initial_condition = 0
  []
  [Tduct]
    block = duct
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  P_out = ${P_out}

  solve = false

  # Solver parameters
  n_blocks = 10
  implicit = false
  segregated = true
  staggered_pressure = false

  # Tolerances
  P_tol = 1.0e-4
  T_tol = 1.0e-8

  # Output
  compute_density = true
  compute_viscosity = true
  compute_power = true
  verbose_multiapps = true
  verbose_subchannel = false

  # friction model
  friction_closure = 'cheng'

  # HTC
  pin_HTC_closure = Dittus-Boelter
  duct_HTC_closure = Dittus-Boelter

  # mixing model (beta)
  mixing_closure = 'cheng_todreas'
[]

[SCMClosures]
  [Cheng]
    type = SCMFrictionUpdatedChengTodreas
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
  [cheng_todreas]
    type = SCMMixingChengTodreas
  []
[]

[SCMClosures]
  [cheng]
    type = SCMFrictionUpdatedChengTodreas
  []
[]

[Executioner]
  type = Steady
[]


[Positions]
  [pin_positions]
    type = SCMPinPositions
    outputs = json
  []
[]

[Outputs]
  json = true
[]
