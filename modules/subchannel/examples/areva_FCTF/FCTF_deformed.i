# Following Benchmark Specifications and Data Requirements for the AREVA heated-bundle test in its Fuel Cooling Test Facility (FCTF)
# as part of a U.S. DOE funded project: Towards a Longer-Life Core. In partnership with TerraPower, TAMU and ANL,
# AREVA NP tested a wire-wrapped rod bundle. The bundle consists of electrically heated pins and non-heated pins.
# This test collected measurements to evaluate thermal hydraulic performance of a wire wrapped bundle, useful for CFD and other software validation.
# Available at: https://www.osti.gov/servlets/purl/1346027/
###################################################
# Steady state subchannel calculation
# Thermal-hydraulics parameters
###################################################
T_in = 305.68 #Kelvin (32.53 C)
Total_Surface_Area_SC = 0.00285294 #m2
Total_Surface_Area_EXP = 0.002808 #m2
P_out = 829370.355 # Pa (120.29 psia)
Power = 90640 # Watt Each heater rod had a max power of 30kW
# Heater 17 (18) not working.
# test:19 power = 22613 22610 22754 22663 [W], Total Power = 90640 [W], mdot_average = 9.576 [kg/s], Re = 20300
# Index of heated pins per silicon controled rectifiers (Areva notation):1 3 6 7 || 4 5 11 15 ||2 9 19 40 60 || 13 44 48 52 56 (from bottom to top)
# Index of heated pins per silicon controled rectifiers (SC notation):0 3 6 1 || 4 5 12 16 || 2 10 8 43 39 || 14 47 51 55 59 (from top to bottom) 38 areva->41 SC
# Relative power of pin per rectifier: 1.12266659312 || 1.12251765225 || 0.90373345101 || 0.90011915269
mdot_average = '${fparse 9.58 * Total_Surface_Area_SC / Total_Surface_Area_EXP}'
mass_flux_in = '${fparse mdot_average / Total_Surface_Area_SC}' #kg/m2
###################################################
# Geometric parameters
###################################################
fuel_pin_pitch = 0.01122652 #m
fuel_pin_diameter = 0.009514 #m
wire_z_spacing = 0.285 #m
wire_diameter = 0.0017062 #m
inner_duct_in = 0.092 #m
n_rings = 5
unheated_length_entry = 1.14 #m 
heated_length = 1.71 #m 
unheated_length_exit = 0.855 #m 
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = ${n_rings}
    n_cells = 100
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
    n_cells = 100
    unheated_length_entry = ${unheated_length_entry}
    heated_length = ${heated_length}
    unheated_length_exit = ${unheated_length_exit}
    pitch = ${fuel_pin_pitch}
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    expression = '(0.4*pi/(pi-2))*sin(pi*z/L) + 1.4 - (0.4*pi/(pi-2))'
    symbol_names = 'L'
    symbol_values = '${heated_length}'
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

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[Problem]
  type = LiquidMetalSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  P_out = ${P_out}
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-4
  implicit = true
  segregated = false
  interpolation_scheme = 'upwind'
  verbose_subchannel = false
[]

# [Problem]
#   type = NoSolveProblem
# []

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
    fp = water
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = water
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = water
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []

  [displacement_ic]
    type = FCTFdisplacementIC
    variable = displacement
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

### TC's on duct face E correspond to subchannels: 122, 121, 123, 123-124, 124, 125
### heights [mm]: 40.2 (centered on A (104-103) & E(123-124)), 2005.2, 2052.7, 2409.0, 3074.0 

[Postprocessors]
  ##### -4 pitch from heated entrance 
  [Pressure_B04_109]
    type = SubChannelPointValue
    variable = P
    index = 109
    execute_on = 'TIMESTEP_END'
    height = 0.0
  []
  ##### -3 pitch from heated entrance 
  [Pressure_B03_109]
    type = SubChannelPointValue
    variable = P
    index = 109
    execute_on = 'TIMESTEP_END'
    height = 0.285
  []
  ##### -2 pitch from heated entrance 
  [Pressure_B02_109]
    type = SubChannelPointValue
    variable = P
    index = 109
    execute_on = 'TIMESTEP_END'
    height = 0.57
  []
  ##### -1.5 pitch from heated entrance 
  [Pressure_B015_109]
    type = SubChannelPointValue
    variable = P
    index = 109
    execute_on = 'TIMESTEP_END'
    height = 0.7125
  []
  ##### -0.5 pitch from heated entrance 
  [Pressure_B05_109]
    type = SubChannelPointValue
    variable = P
    index = 109
    execute_on = 'TIMESTEP_END'
    height = 0.9975
  []
  ##### 0 pitch from heated entrance 
  [Pressure_B0_109]
    type = SubChannelPointValue
    variable = P
    index = 109
    execute_on = 'TIMESTEP_END'
    height = 1.14
  []
  #### 3 pitch from heated entrance
  [Pressure_B3_109]
    type = SubChannelPointValue
    variable = P
    index = 109
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  #### 6 pitch from heated entrance
  [Pressure_B6_109]
    type = SubChannelPointValue
    variable = P
    index = 109
    execute_on = 'TIMESTEP_END'
    height = 2.85
  []
  #### 8 pitch from heated entrance
  [Pressure_B8_109]
    type = SubChannelPointValue
    variable = P
    index = 109
    execute_on = 'TIMESTEP_END'
    height = 3.42
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
    input_files = '3D.i'
    execute_on = 'FINAL'
  []
[]

[Transfers]
  [subchannel_transfer]
    type = MultiAppDetailedSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S displacement'
  []
  [pin_transfer]
    type = MultiAppDetailedPinSolutionTransfer
    to_multi_app = viz
    variable = 'Dpin Tpin q_prime'
  []
[]
