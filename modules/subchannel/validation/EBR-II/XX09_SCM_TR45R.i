# Following Benchmark Specifications and Data Requirements for EBR-II Shutdown Heat Removal Tests SHRT-17 and SHRT-45R
# Available at: https://publications.anl.gov/anlpubs/2012/06/73647.pdf
# Transient Subchannel calculation
###################################################
# Thermal-hydraulics parameters
###################################################
T_in = 616.4 #Kelvin
Total_Surface_Area = 0.000854322 #m3
mass_flux_in = '${fparse 2.427 / Total_Surface_Area}'
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
  [q_prime_init]
    block = fuel_pins
  []
  [power_history_field]
    block = fuel_pins
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
  T_tol = 1.0e-4
  implicit = true
  segregated = false
  interpolation_scheme = 'upwind'

  # Heat Transfer Correlations
  pin_htc_correlation = 'gnielinski'
  duct_htc_correlation = 'gnielinski'
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
    variable = q_prime_init
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

[Functions]
  [power_func]
    type = PiecewiseLinear
    data_file = 'power_history_SHRT45.csv'
    format = "columns"
    scale_factor = 1.0
  []
  [mass_flux_in]
    type = PiecewiseLinear
    data_file = 'massflow_SHRT45.csv'
    format = "columns"
    scale_factor = '${fparse mass_flux_in / 2.427}'
  []
  [dts]
    type = PiecewiseLinear
    xy_data = '0.0 0.1
               5.0 2.0
               100 2.0
               110 20.0
               900 20.0'
  []
[]

[Controls]
  [mass_flux_ctrl]
    type = RealFunctionControl
    parameter = 'Postprocessors/mass_flux_PP/value'
    function = 'mass_flux_in'
    execute_on = 'initial timestep_begin'
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
    mass_flux = mass_flux_PP
    execute_on = 'timestep_begin'
  []
  [populate_power_history]
    type = FunctionAux
    variable = power_history_field
    function = 'power_func'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [change_q_prime]
    type = ParsedAux
    variable = q_prime
    coupled_variables = 'q_prime_init power_history_field'
    expression = 'q_prime_init*power_history_field'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [report_pressure_outlet]
    type = Receiver
    default = ${P_out}
  []

  [TTC-31]
    type = SubChannelPointValue
    variable = T
    index = 0
    execute_on = 'initial timestep_end'
    height = 0.322
  []

  [post_func]
    type = ElementIntegralVariablePostprocessor
    block = fuel_pins
    variable = q_prime
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []

  [mass_flux_PP]
    type = ConstantPostprocessor
    value = ${mass_flux_in}
  []

  [mass_flow_PP]
    type = ParsedPostprocessor
    expression = '${Total_Surface_Area} * mass_flux_PP'
    pp_names = 'mass_flux_PP'
  []
[]

[Executioner]
  type = Transient

  start_time = -1
  end_time = 900.0
  [TimeStepper]
    type = FunctionDT
    function = dts
    min_dt = 0.1
    growth_factor = 2.0
  []
  dtmax = 20
  # num_steps = 15
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################
[MultiApps]
  [viz]
    type = TransientMultiApp
    input_files = '3d_SCM_TR.i'
    execute_on = 'INITIAL TIMESTEP_END'
    catch_up = true
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
