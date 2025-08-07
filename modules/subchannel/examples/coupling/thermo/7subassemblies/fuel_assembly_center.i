###################################################
# Thermal-hydraulics parameters
###################################################
T_in = 653.15
P_out = 758423 # Pa
fuel_assemblies_per_power_unit = '${fparse 180}'
flow_area = 0.00650277
mass_flux_in = '${fparse 1256*4/fuel_assemblies_per_power_unit/flow_area}' # kg/(m2.s)

###################################################
# Geometric parameters
###################################################
#f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
scale_factor = 0.01
fuel_element_pitch = '${fparse 16.2471*scale_factor}'
inter_assembly_gap = '${fparse 0.4348*scale_factor}' # check
duct_thickness = '${fparse 0.3966*scale_factor}'
fuel_pin_pitch = '${fparse 0.8966*scale_factor}'
fuel_pin_diameter = '${fparse 0.7714*scale_factor}'
wire_z_spacing = '${fparse 20.43*scale_factor}' ###
wire_diameter = '${fparse 0.1307*scale_factor}' #check
n_rings = 10
length_entry_fuel = '${fparse 160.92*scale_factor}'
length_heated_fuel = '${fparse 85.82*scale_factor}'
length_outlet_fuel = '${fparse 233.46*scale_factor}'
orifice_plate_height = '${fparse 5*scale_factor}'
duct_outside = '${fparse fuel_element_pitch - inter_assembly_gap}'
duct_inside = '${fparse duct_outside - 2 * duct_thickness}'
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = 50
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
    n_cells = 50 #100
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
  []

  [duct]
    type = SCMTriDuctMeshGenerator
    input = fuel_pins
    nrings = '${fparse n_rings}'
    n_cells = 50 #100
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    expression = '(pi/2)*sin(pi*z/L)'
    symbol_names = 'L'
    symbol_values = '${length_heated_fuel}'
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
  [q_prime_duct]
    block = duct
  []
  [Tduct]
    block = duct
    initial_condition = 653.15
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
  n_blocks = 1
  P_out = ${P_out}
  CT = 1.0
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-3
  implicit = true
  segregated = false
  verbose_multiapps = true
  verbose_subchannel = true
[]

[ICs]
  [S_IC]
    type = SCMTriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMTriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = SCMTriPowerIC
    variable = q_prime
    power = 2000000.0 #W
    filename = "pin_power_profile_uni.txt"
    axial_heat_rate = axial_heat_rate
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [P_ic]
    type = ConstantIC
    variable = P
    value = 0.0 #always set to zero
  []

  [DP_ic]
    type = ConstantIC
    variable = DP
    value = 0.0
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = ${fuel_pin_diameter}
  []

  [Viscosity_ic]
    type = ViscosityIC
    variable = mu
    p = ${P_out}
    T = T
    fp = sodium
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = sodium
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[AuxKernels]
  [P_out_bc]
    type = ConstantAux
    variable = P
    boundary = outlet
    value = ${P_out}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [mdot_in_bc]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################
[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = "3d.i"
    execute_on = "final"
    output_in_position = true
  []
[]

[Transfers]
  [subchannel_transfer]
    type = SCMSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S w_perim'
  []

  [pin_transfer]
    type = SCMPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin Dpin q_prime'
  []
[]
