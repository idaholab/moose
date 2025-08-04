# Following Benchmark Specifications and Data Requirements for EBR-II Shutdown Heat Removal Tests SHRT-17 and SHRT-45R
# Available at: https://publications.anl.gov/anlpubs/2012/06/73647.pdf
###################################################
# Steady state subchannel calcultion
# Thermal-hydraulics parameters
###################################################
T_in = 616.4 #Kelvin
Total_Surface_Area = 0.000854322 #m2
Mass_In = 2.427 #kg/sec
mass_flux_in = '${fparse Mass_In / Total_Surface_Area}' #kg/m2
P_out = 2.0e5
Power_initial = 379800 #W (Page 26,35 of ANL document)
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

  [duct]
    type = SCMTriDuctMeshGenerator
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
    value = '(pi/2)*sin(pi*z/L)*exp(-alpha*z)/(1.0/alpha*(1.0 - exp(-alpha*L)))*L'
    vars = 'L alpha'
    vals = '${heated_length} 1.8012'
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
  [q_prime_duct]
    block = duct
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
  [MTC-20]
  type = SubChannelPointValue
  variable = T
  index = 33
  execute_on = 'TIMESTEP_END'
  height = 0.172
  []
  [MTC-22]
    type = SubChannelPointValue
    variable = T
    index = 3
    execute_on = 'TIMESTEP_END'
    height = 0.172
  []
  [MTC-24]
    type = SubChannelPointValue
    variable = T
    index = 28
    execute_on = 'TIMESTEP_END'
    height = 0.172
  []
  [MTC-25]
    type = SubChannelPointValue
    variable = T
    index = 60
    execute_on = 'TIMESTEP_END'
    height = 0.172
  []
  [MTC-26]
    type = SubChannelPointValue
    variable = T
    index = 106
    execute_on = 'TIMESTEP_END'
    height = 0.172
  []
  [14TC-37]
    type = SubChannelPointValue
    variable = T
    index = 52
    execute_on = 'TIMESTEP_END'
    height = 0.480
  []
  [14TC-39]
    type = SubChannelPointValue
    variable = T
    index = 6
    execute_on = 'TIMESTEP_END'
    height = 0.480
  []
  [14TC-41]
    type = SubChannelPointValue
    variable = T
    index = 40
    execute_on = 'TIMESTEP_END'
    height = 0.480
  []
  [14TC-43]
    type = SubChannelPointValue
    variable = T
    index = 105
    execute_on = 'TIMESTEP_END'
    height = 0.480
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
    input_files = '3d_SCM_SS.i'
    execute_on = 'FINAL'
  []
[]

[Transfers]
  [subchannel_transfer]
    type = SCMSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
  []
  [pin_transfer]
    type = SCMPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin q_prime'
  []
[]
