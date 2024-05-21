# Following Benchmark Specifications and Data Requirements for the AREVA heated-bundle test in its Fuel Cooling Test Facility (FCTF)
# as part of a U.S. DOE funded project: Towards a Longer-Life Core. In partnership with TerraPower, TAMU and ANL,
# AREVA NP tested a wire-wrapped rod bundle. The bundle consists of electrically heated pins and non-heated pins.
# This test collected measurements to evaluate thermal hydraulic performance of a wire wrapped bundle, useful for CFD and other software validation.
# Available at: https://www.osti.gov/servlets/purl/1346027/
###################################################
# Steady state subchannel calculation
# Thermal-hydraulics parameters
###################################################
T_in = 305.44 #Kelvin (32.29 C)
# mu = 0.0007646 #Pas
# Re = 20500
# Dh = 0.004535
Total_Surface_Area_SC = 0.00285294 #m2
Total_Surface_Area_EXP = 0.002808 #m2
P_out = 829370.355 # Pa (120.29 psia)
Power = 90640 # Watt Each heater rod had a max power of 30kW
# Heater 17 (18) not working.
# test:19 power = 22613 22610 22754 22663 [W], Total Power = 90640 [W], mdot_average = 9.576 [kg/s], Re = 20300
# Index of heated pins per silicon controled rectifiers (Areva notation):1 3 6 7 || 4 5 11 15 ||2 9 19 40 60 || 13 44 48 52 56 (from bottom to top)
# Index of heated pins per silicon controled rectifiers (SC notation):0 3 6 1 || 4 5 12 16 || 2 10 8 43 39 || 14 47 51 55 59 (from top to bottom) 38 areva->41 SC
# Relative power of pin per rectifier: 1.12266659312 || 1.12251765225 || 0.90373345101 || 0.90011915269
mdot_average = '${fparse 9.33 * Total_Surface_Area_SC / Total_Surface_Area_EXP}'
mass_flux_in = '${fparse mdot_average / Total_Surface_Area_SC}' #kg/m2
###################################################
# Geometric parameters (non-deformed heated bundle)
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
    n_cells = 65
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
    n_cells = 65
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
  type = TriSubChannel1PhaseProblem
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
  verbose_subchannel = true
  deformation = false
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
  ##### Pressure on Face B at various axial heights
  ##### -4 pitch from heated entrance
  [Pressure_FB01]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.0
  []
  ##### -3 pitch from heated entrance
  [Pressure_FB02]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.285
  []
  ##### -2 pitch from heated entrance
  [Pressure_FB03]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.57
  []
  ##### -1.5 pitch from heated entrance
  [Pressure_FB04]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.7125
  []
  ##### -0.5 pitch from heated entrance
  [Pressure_FB05]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.9975
  []
  ##### 0 pitch from heated entrance
  [Pressure_FB06]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 1.14
  []
  ##### 0.5 pitch from heated entrance
  [Pressure_FB07]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 1.2825
  []
  #### 3 pitch from heated entrance
  [Pressure_FB08]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  #### 6 pitch from heated entrance
  [Pressure_FB09]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 2.85
  []
  #### 8 pitch from heated entrance
  [Pressure_FB10]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 3.42
  []
  ###### Temperature at 4' and 6' o clock at plane B (SC) ##########
  ###### Plane B is 4.4167 P above start of heated section
  ###### Plane B: z = 2.3987595
  # [Temp_B_01]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 14
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_02]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 28
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_03]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 45
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_04]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 47
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_05]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 33
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_06]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 23
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_07]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 39
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_08]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 62
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_09]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 86
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_10]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 66
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_11]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 51
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_12]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 77
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_13]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 79
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_14]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 106
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_15]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 112
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_16]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 111
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_17]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 117
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_18]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 119
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_19]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 120
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_20]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 95
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_21]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 54
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_22]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 56
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # ########### Planar mean of temperature at plane B
  # [Temp_B]
  #   type = PlanarMean
  #   variable = T
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # ####### Temperature at wall E at plane B
  # [Temp_B_E1]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 121
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_E2]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 123
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_E4]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 124
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_B_E5]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 125
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # ###### Temperature at 8 and 10 'o clock at plane C ##########
  # ###### Plane C is 6.75 P above start of heated section
  # ###### Plane C: z = 3.06375 [m]
  [Temp_C_01]
    type = SubChannelPointValue
    variable = T
    index = 25
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_02]
    type = SubChannelPointValue
    variable = T
    index = 15
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_03]
    type = SubChannelPointValue
    variable = T
    index = 18
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_04]
    type = SubChannelPointValue
    variable = T
    index = 32
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_05]
    type = SubChannelPointValue
    variable = T
    index = 51
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_06]
    type = SubChannelPointValue
    variable = T
    index = 53
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_07]
    type = SubChannelPointValue
    variable = T
    index = 56
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_C_08]
    type = SubChannelPointValue
    variable = T
    index = 29
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_09]
    type = SubChannelPointValue
    variable = T
    index = 47
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_10]
    type = SubChannelPointValue
    variable = T
    index = 67
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_11]
    type = SubChannelPointValue
    variable = T
    index = 93
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_12]
    type = SubChannelPointValue
    variable = T
    index = 76
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_13]
    type = SubChannelPointValue
    variable = T
    index = 78
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_14]
    type = SubChannelPointValue
    variable = T
    index = 79
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_15]
    type = SubChannelPointValue
    variable = T
    index = 84
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_16]
    type = SubChannelPointValue
    variable = T
    index = 85
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_17]
    type = SubChannelPointValue
    variable = T
    index = 116
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_18]
    type = SubChannelPointValue
    variable = T
    index = 120
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_19]
    type = SubChannelPointValue
    variable = T
    index = 122
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_20]
    type = SubChannelPointValue
    variable = T
    index = 97
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_21]
    type = SubChannelPointValue
    variable = T
    index = 96
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_22]
    type = SubChannelPointValue
    variable = T
    index = 99
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  ########### Planar mean of temperature at plane C
  [Temp_C]
    type = PlanarMean
    variable = T
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  ####### Temperature at wall E at plane B
  [Temp_C_E1]
    type = SubChannelPointValue
    variable = T
    index = 121
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_E2]
    type = SubChannelPointValue
    variable = T
    index = 123
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_E4]
    type = SubChannelPointValue
    variable = T
    index = 124
    execute_on = 'TIMESTEP_END'
    height = 3.06375
  []
  [Temp_C_E5]
    type = SubChannelPointValue
    variable = T
    index = 125
    execute_on = 'TIMESTEP_END'
    height = 3.06375
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
