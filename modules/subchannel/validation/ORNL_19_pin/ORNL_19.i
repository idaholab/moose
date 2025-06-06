# M. Fontana, et All,
# "Temperature distribution in the duct wall and at the exit of a 19-pin simulated lmfbr fuel assembly (ffm bundle 2a),
# "Nuclear Technology, vol. 24, no. 2, pp. 176-200, 1974.
T_in = 588.71 #600F
A12 = 1.00423e3
A13 = -0.21390
A14 = -1.1046e-5
rho = '${fparse A12 + A13 * T_in + A14 * T_in * T_in}'
flow_area = 0.000467906 #m2
vol_flow = 3.47E-03 #m^3/sec
mass_flux_in = '${fparse rho *  vol_flow / flow_area}'
P_out = 2.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = 3
    n_cells = 40
    flat_to_flat = 3.41e-2
    heated_length = 0.5334
    unheated_length_entry = 0.4064
    unheated_length_exit = 0.0762
    pin_diameter = 5.84e-3
    pitch = 7.26e-3
    dwire = 1.42e-3
    hwire = 0.3048
    spacer_z = '0'
    spacer_k = '0'
  []
[]

[AuxVariables]
  [mdot]
  []
  [SumWij]
  []
  [P]
  []
  [DP]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
  [mu]
  []
  [S]
  []
  [w_perim]
  []
  [q_prime]
  []
  [displacement]
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
  P_out = 2.0e5
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
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
    power = 322525.0 #W
    filename = "pin_power_profile19.txt"
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
  csv = true
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [T1]
    type = SubChannelPointValue
    variable = T
    index = 37
    execute_on = "timestep_end"
    height = 1.016
  []
  [T2]
    type = SubChannelPointValue
    variable = T
    index = 36
    execute_on = "timestep_end"
    height = 1.016
  []
  [T3]
    type = SubChannelPointValue
    variable = T
    index = 20
    execute_on = "timestep_end"
    height = 1.016
  []
  [T4]
    type = SubChannelPointValue
    variable = T
    index = 10
    execute_on = "timestep_end"
    height = 1.016
  []
  [T5]
    type = SubChannelPointValue
    variable = T
    index = 4
    execute_on = "timestep_end"
    height = 1.016
  []
  [T6]
    type = SubChannelPointValue
    variable = T
    index = 1
    execute_on = "timestep_end"
    height = 1.016
  []
  [T7]
    type = SubChannelPointValue
    variable = T
    index = 14
    execute_on = "timestep_end"
    height = 1.016
  []
  [T8]
    type = SubChannelPointValue
    variable = T
    index = 28
    execute_on = "timestep_end"
    height = 1.016
  []
  ####### Assembly pressure drop
  [DP_SubchannelDelta]
    type = SubChannelDelta
    variable = P
    execute_on = 'TIMESTEP_END'
  []
  #####
  [Mean_Temp]
    type = SCMPlanarMean
    variable = T
    height = 2
  []

  [Total_power]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime
  []
[]
################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = "3d_ORNL_19.i"
    execute_on = "timestep_end"
  []
[]

[Transfers]
  [xfer]
    type = SCMSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu q_prime S'
  []
[]
