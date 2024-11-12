# Based on M. Fontana, et All, this arbitary subassembly is used for THM-SC coupling
T_in = 583.0 #K
flow_area = 0.0004980799633447909 #m2
mass_flux_in = '${fparse 1.0/flow_area}'
P_out = 2e5 # Pa
###################################################
# Geometric parameters
###################################################
n_cells = 25
n_rings = 3
fuel_pin_pitch = 7.26e-3
fuel_pin_diameter = 5.84e-3
wire_z_spacing = 0.3048
wire_diameter = 1.42e-3
inner_duct_in = 3.41e-2
heated_length = 1.0
###################################################
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = ${n_rings}
    n_cells = ${n_cells}
    flat_to_flat = ${inner_duct_in}
    heated_length = ${heated_length}
    pin_diameter = ${fuel_pin_diameter}
    pitch = ${fuel_pin_pitch}
    dwire = ${wire_diameter}
    hwire = ${wire_z_spacing}
    spacer_z = '0.0'
    spacer_k = '0.0'
  []

  [fuel_pins]
    type = TriPinMeshGenerator
    input = subchannel
    nrings = ${n_rings}
    n_cells = ${n_cells}
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
  [w_perim]
    block = subchannel
  []
  [mu]
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

[FluidProperties]
  [Sodium]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = TriSubChannel1PhaseProblem
  fp = Sodium
  n_blocks = 1
  P_out = report_pressure_outlet
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-3
  T_tol = 1.0e-3
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
  verbose_subchannel = false
  interpolation_scheme = 'upwind'
[]

[ICs]
  [S_IC]
    type = SCMTriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = TriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = SCMTriPowerIC
    variable = q_prime
    power = 10000 #W
    filename = "pin_power_profile19.txt"
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
    fp = Sodium
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = Sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = Sodium
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[AuxKernels]
  [T_in_bc]
    type = PostprocessorConstantAux
    variable = T
    boundary = inlet
    postprocessor = report_temperature_inlet
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [mdot_in_bc]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = report_mass_flux_inlet
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

[Postprocessors]
  [total_pressure_drop_SC]
    type = SubChannelDelta
    variable = P
    execute_on = "timestep_end"
  []

  [total_pressure_drop_SC_limited]
    type = ParsedPostprocessor
    pp_names = 'total_pressure_drop_SC'
    function = 'min(total_pressure_drop_SC, 1e6)'
    execute_on = "timestep_end"
  []

  [Total_power]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime
    block = fuel_pins
  []

  [report_mass_flux_inlet]
    type = Receiver
    default = ${mass_flux_in}
  []

  [report_temperature_inlet]
    type = Receiver
    default = ${T_in}
  []

  [report_pressure_outlet]
    type = Receiver
    default = ${P_out}
  []
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################
[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = '3D.i'
    execute_on = 'FINAL'
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
    variable = 'Dpin Tpin q_prime'
  []
[]
