# Following Benchmark Specifications and Data Requirements for EBR-II Shutdown Heat Removal Tests SHRT-17 and SHRT-45R
# Available at: https://publications.anl.gov/anlpubs/2012/06/73647.pdf
###################################################
#Steady state subchannel calcultion,with adapted massflow rate
# Thermal-hydraulics parameters
###################################################
T_in = 624.70556 #Kelvin
Total_Surface_Area = 0.000854322 #m3
mass_flux_in = '${fparse 2.6923 / Total_Surface_Area}' #
P_out = 2.0e5
Power_initial = 486200 #W (Page 26,35 of ANL document)
###################################################
# Geometric parameters
###################################################
scale_factor = 0.01
fuel_pin_pitch = '${fparse 0.5664*scale_factor}'
fuel_pin_diameter = '${fparse 0.4419*scale_factor}'
wire_z_spacing = '${fparse 15.24*scale_factor}'
wire_diameter = '${fparse 0.1244*scale_factor}'
inner_duct_in = '${fparse 4.64*scale_factor}'
n_rings = 5
heated_length = '${fparse 34.3*scale_factor}'
unheated_length_exit = '${fparse 26.9*scale_factor}'
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = ${n_rings}
    n_cells = 50
    flat_to_flat = ${inner_duct_in}
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
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
    n_cells = 50
    unheated_length_exit = ${unheated_length_exit}
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
  [q_prime]
    block = fuel_pins
  []
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
    block = fuel_pins
  []
  [displacement]
    block = subchannel
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
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-5
  implicit = true
  segregated = false
  interpolation_scheme = 'upwind'
  deformation = true
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
    power = ${Power_initial}
    filename = "pin_power_profile61_uniform.txt"
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
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
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
[]

[Outputs]
  exodus = true
  csv = true
[]

[Executioner]
  type = Steady
[]
