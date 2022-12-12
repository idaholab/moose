T_in = 302.594
mass_flux_in = 1730.0950134985335
P_out = 101325  # Pa
# Creer et. al 1976
# Blockage is modeled with area reduction and form loss coefficient distributed on the
# affected subchannels

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 8
    ny = 8
    n_cells = 36
    pitch = 0.0136906
    Kij = 0.5
    rod_diameter = 0.0099568
    gap = 0.0036957
    heated_length = 1.4224
    z_blockage = '0.60325 0.67945'
    index_blockage = '18 19 20 21 26 27 28 29 34 35 36 37 42 43 44 45'
    reduction_blockage = '0.83 0.65 0.65 0.83 0.65 0.30 0.30 0.65 0.65 0.30 0.30 0.65 0.83 0.65 0.65 0.83'
    k_blockage = '0.0 0.0 0.0 0.0 0.0 0.2 0.2 0.0 0.0 0.2 0.2 0.0 0.0 0.0 0.0 0.0'
    spacer_z = '0.3683 1.3843'
    spacer_k = '1.14 1.14'
  []

#   [fuel_pins]
#     type = QuadPinMeshGenerator
#     input = sub_channel
#     nx = 8
#     ny = 8
#     n_cells = 20
#     pitch = 0.0136906
#     heated_length = 1.4478
#   []
[]

[AuxVariables]
  [mdot]
    block = sub_channel
  []
  [SumWij]
    block = sub_channel
  []
  [P]
    block = sub_channel
  []
  [DP]
    block = sub_channel
  []
  [h]
    block = sub_channel
  []
  [T]
    block = sub_channel
  []
#   [Tpin]
#     block = fuel_pins
#   []
  [rho]
    block = sub_channel
  []
  [mu]
    block = sub_channel
  []
  [S]
    block = sub_channel
  []
  [w_perim]
    block = sub_channel
  []
#   [q_prime]
#     block = fuel_pins
#   []
[]

[Modules]
  [FluidProperties]
    [water]
      type = Water97FluidProperties
    []
  []
[]

[SubChannel]
  type = LiquidWaterSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  beta = 0.006
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = false
  P_out = ${P_out}
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
  interpolation_scheme = 'exponential'
[]

[ICs]
  [S_IC]
    type = QuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = QuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = QuadPowerIC
    variable = q_prime
    power = 0.0 # W
    filename = "power_profile.txt"
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
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
  [Surface_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = S
    execute_on = final
    file_base = "Surface.txt"
    height = 1.0
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]


[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = "detailedMesh.i"
    execute_on = "final"
  []
[]

[Transfers]
  ###### Transfers to the detailedMesh at the end of the coupled simulations
  [subchannel_transfer]
    type = MultiAppDetailedSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
  []
#   [pin_transfer]
#     type = MultiAppDetailedPinSolutionTransfer
#     to_multi_app = viz
#     variable = 'Tpin q_prime'
#   []
[]

