# Following Benchmark Specifications and Data Requirements for the AREVA heated-bundle test in its Fuel Cooling Test Facility (FCTF)
# as part of a U.S. DOE funded project: Towards a Longer-Life Core. In partnership with TerraPower, TAMU and ANL,
# AREVA NP tested a wire-wrapped rod bundle. The bundle consists of electrically heated pins and non-heated pins.
# This test collected measurements to evaluate thermal hydraulic performance of a wire wrapped bundle, useful for CFD and other software validation.
# Available at: https://www.osti.gov/servlets/purl/1346027/
###################################################
# Steady state subchannel calculation
# Thermal-hydraulics parameters
###################################################
T_in = ??? #Kelvin
Total_Surface_Area = 0.002808 #m2
Dh = 0.004535 #m
Re = 20300 # [-] (17000 20100 17000 20300)
# mdot_average (12.01 9.533 11.996 9.576)
P_out = 827371 # Pa
Power = ??? # Watt Each heater rod had a max power of 30kW
# Heater 17 (12) not working.
# test:05 power = 22643 22845 22747 22801 [W], Total Power = 91036 [W], mdot_average = 12.01 [kg/s], Re = 17000
# test:19 power = 22612 22610 22754 22663 [W], Total Power = 90639 [W], mdot_average = 9.576 [kg/s], Re = 20300
# Index of heated pins per silicon controled rectifiers (Areva notation):2 3 6 7 || 4 5 11 15 ||1 9 19 40 60 || 13 44 48 52 56
# Index of heated pins per silicon controled rectifiers (SC notation): 2 1 4 3 || 6 5 18 14 || 0 8 10 39 43 || 16 59 55 51 47
mass_flux_in = mdot_average / Total_Surface_Area #kg/m2
###################################################
# Geometric parameters
###################################################
# Total heater lehgth : 5.08 m
fuel_pin_pitch = 0.01125 #m
fuel_pin_diameter = 0.0095 #m
wire_z_spacing = 0.285 #m
wire_diameter = 0.00172 #m
inner_duct_in = 0.09164 #m
n_rings = 5
unheated_length_entry = 1.95 #m
heated_length = 1.71 #m
unheated_length_exit = 1.42 #m
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = ${n_rings}
    n_cells = 50
    flat_to_flat = ${inner_duct_in}
    unheated_length_entry = ${unheated_length_entry}
    heated_length = ${heated_length}
    unheated_length_exit = ${unheated_length_exit}
    rod_diameter = ${fuel_pin_diameter}
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
    n_cells = 50
    unheated_length_entry = ${unheated_length_entry}
    heated_length = ${heated_length}
    unheated_length_exit = ${unheated_length_exit}
    pitch = ${fuel_pin_pitch}
  []

  [duct]
    type = TriDuctMeshGenerator
    input = fuel_pins
    nrings = ${n_rings}
    n_cells = 50
    flat_to_flat = ${inner_duct_in}
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
    pitch = ${fuel_pin_pitch}
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = '(0.4*pi/(pi-2))*sin(pi*z/L) + 1.4 - (0.4*pi/(pi-2))'
    vars = 'L'
    vals = '${heated_length}'
  []

  [duct_deformation]
    type = ParsedFunction
    value = '0.001064*sin(pi*z/L)'
    vars = 'L'
    vals = '${heated_length}'
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
  P_out = ${P_out}
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-5
  implicit = true
  segregated = false
  interpolation_scheme = 'upwind'
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
    power = ${Power}
    filename = "pin_power_profile61.txt"
    axial_heat_rate = axial_heat_rate
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

  [displacement_ic]
    type = ConstantIC
    variable = displacement
    value =
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
    type = MassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [TTC-27]
    type = SubChannelPointValue
    variable = T
    index = 91
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-28]
    type = SubChannelPointValue
    variable = T
    index = 50
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-29]
    type = SubChannelPointValue
    variable = T
    index = 21
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-30]
    type = SubChannelPointValue
    variable = T
    index = 4
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-31]
    type = SubChannelPointValue
    variable = T
    index = 2
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-32]
    type = SubChannelPointValue
    variable = T
    index = 16
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-33]
    type = SubChannelPointValue
    variable = T
    index = 42
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-34]
    type = SubChannelPointValue
    variable = T
    index = 80
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-35]
    type = SubChannelPointValue
    variable = T
    index = 107
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  # [MTC-20]
  # type = SubChannelPointValue
  # variable = T
  # index = 33
  # execute_on = 'TIMESTEP_END'
  # height = 0.172
  # []
  # [MTC-22]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 3
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.172
  # []
  # [MTC-24]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 28
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.172
  # []
  # [MTC-25]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 60
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.172
  # []
  # [MTC-26]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 106
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.172
  # []
  # [14TC-37]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 52
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.480
  # []
  # [14TC-39]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 6
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.480
  # []
  # [14TC-41]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 40
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.480
  # []
  # [14TC-43]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 105
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.480
  # []
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
    input_files = '3d_SC_ss.i'
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
    variable = 'Tpin q_prime'
  []
[]
