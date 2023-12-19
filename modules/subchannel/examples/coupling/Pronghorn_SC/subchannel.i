# Problems to be fixed:
# 1. subchannel solver randomness might affect coupling:
# Take too much time to converge or not solve at all. Implicit solver is more robust especially for
# liquid metal hexagonal assemblies.
# 2. parallel execution of subchannel

# Following Advanced Burner Test Reactor Preconceptual Design Report
# Vailable at: https://www.ne.anl.gov/eda/ABTR_1cv2_ws.pdf
###################################################
# Thermal-hydraulics parameters
###################################################
T_in = 628.15
P_out = 758423 # Pa
mass_flux_in = '${fparse 2786}' # kg/(m2.s)

###################################################
# Geometric parameters
###################################################

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
length_entry_fuel = '${fparse 60*scale_factor}'
length_heated_fuel = '${fparse 80*scale_factor}'
length_outlet_fuel = '${fparse 120*scale_factor}'
orifice_plate_height = '${fparse 5*scale_factor}'
duct_outside = '${fparse fuel_element_pitch - inter_assembly_gap}'
duct_inside = '${fparse duct_outside - 2 * duct_thickness}'
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = 26
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    rod_diameter = '${fparse fuel_pin_diameter}'
    pitch = '${fparse fuel_pin_pitch}'
    dwire = '${fparse wire_diameter}'
    hwire = '${fparse wire_z_spacing}'
    spacer_z = '${fparse orifice_plate_height} ${fparse length_entry_fuel}'
    spacer_k = '0.5 0.5'
  []

  [fuel_pins]
    type = TriPinMeshGenerator
    input = subchannel
    nrings = '${fparse n_rings}'
    n_cells = 26
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
  [Sij]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [q_prime]
    block = fuel_pins
  []
  [mu]
    block = subchannel
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = LiquidMetalSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = report_pressure_outlet
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = false
  P_tol = 1.0e-3
  T_tol = 1.0e-3
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
  verbose_subchannel = false
[]

[ICs]
  [S_IC]
    type = TriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = TriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = TriPowerIC
    variable = q_prime
    power = 0.0
    filename = "pin_power_profile217.txt"
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = ${fuel_pin_diameter}
  []

  [P_ic]
    type = ConstantIC
    variable = P
    value = 0.0
  []

  [DP_ic]
    type = ConstantIC
    variable = DP
    value = 0.0
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
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [mdot_in_bc]
    type = PostprocessorMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    postprocessor = report_mass_flux_inlet
    execute_on = 'timestep_begin'
    block = subchannel
  []
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [total_pressure_drop_SC]
    type = SubChannelDelta
    variable = P
    execute_on = "timestep_end"
  []

  [report_mass_flux_inlet]
    type = Receiver
    default = ${mass_flux_in}
  []

  [report_pressure_outlet]
    type = Receiver
    default = ${P_out}
  []
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
    input_files = '3d_subchannel.i'
    execute_on = 'final'
  []
[]

[Transfers]
  [subchannel_transfer]
    type = MultiAppDetailedSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
  []
  [pin_transfer]
    type = MultiAppDetailedPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin q_prime'
  []
[]
