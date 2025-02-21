# Following Benchmark Specifications and Data Requirements for the AREVA heated-bundle test in its Fuel Cooling Test Facility (FCTF)
# as part of a U.S. DOE funded project: Towards a Longer-Life Core. In partnership with TerraPower, TAMU and ANL,
# AREVA NP tested a wire-wrapped pin bundle. The bundle consists of electrically heated pins and non-heated pins.
# This test collected measurements to evaluate thermal hydraulic performance of a wire wrapped bundle, useful for CFD and other software validation.
# Available at: https://www.osti.gov/servlets/purl/1346027/
###################################################
# This input file tests the ICs kernel that populates discplament for the FCTF deformed assembly.
###################################################
# Steady state subchannel calculation
# Thermal-hydraulics parameters
###################################################
T_in = 305.68 #Kelvin (32.53 C)
Total_Surface_Area_SC = 0.00285294 #m2
Total_Surface_Area_EXP = 0.002808 #m2
P_out = 829370.355 # Pa (120.29 psia)
Power = 90640 # Watt Each heater pin had a max power of 30kW
mdot_average = '${fparse 9.58 * Total_Surface_Area_SC / Total_Surface_Area_EXP}'
mass_flux_in = '${fparse mdot_average / Total_Surface_Area_SC / 1.5}' #kg/m2
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
    type = SCMTriSubChannelMeshGenerator
    nrings = ${n_rings}
    n_cells = 10
    flat_to_flat = ${inner_duct_in}
    unheated_length_entry = ${unheated_length_entry}
    heated_length = ${heated_length}
    unheated_length_exit = ${unheated_length_exit}
    pin_diameter = ${fuel_pin_diameter}
    pitch = ${fuel_pin_pitch}
    dwire = ${wire_diameter}
    hwire = ${wire_z_spacing}
    spacer_z = '0.0'
    spacer_k = '0.0'
  []

  [fuel_pins]
    type = SCMTriPinMeshGenerator
    input = subchannel
    nrings = ${n_rings}
    n_cells = 10
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
  type = NoSolveProblem
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
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
[]

