T_in = 673.15
flow_area = 0.00128171 #m2
rho_in = 10453.21705
# [10 m^3/hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 10*rho_in/3600/flow_area}'
P_out = 1.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = 3
    n_cells = 50
    flat_to_flat = 0.05319936
    heated_length = 0.87
    unheated_length_entry = 0.0
    unheated_length_exit = 0.402
    pin_diameter = 8.2e-3
    pitch = 0.01148
    dwire = 0.0
    hwire = 0.0
    spacer_z = '0.177 0.547 0.870'
    spacer_k = '1.1719 1.1719 1.1719'
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
  [S]
  []
  [w_perim]
  []
  [q_prime]
  []
  [mu]
  []
  [displacement]
  []
[]

[FluidProperties]
  [LBE]
    type = LeadBismuthFluidProperties
  []
[]

[Problem]
  type = TriSubChannel1PhaseProblem
  fp = LBE
  n_blocks = 1
  P_out = 1.0e5
  CT = 1.0
  # enforce_uniform_pressure = false
  compute_density = true
  compute_viscosity = true
  compute_power = true
  implicit = true
  segregated = false
  verbose_subchannel = true
  interpolation_scheme = upwind
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
    power = '${fparse 250000}'
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
    fp = LBE
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = LBE
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = LBE
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

[Postprocessors]
  [T1]
    type = SubChannelPointValue
    variable = T
    index = 37
    execute_on = "timestep_end"
    height = 0.87
  []
  [T2]
    type = SubChannelPointValue
    variable = T
    index = 36
    execute_on = "timestep_end"
    height = 0.87
  []
  [T3]
    type = SubChannelPointValue
    variable = T
    index = 20
    execute_on = "timestep_end"
    height = 0.87
  []
  [T4]
    type = SubChannelPointValue
    variable = T
    index = 10
    execute_on = "timestep_end"
    height = 0.87
  []
  [T5]
    type = SubChannelPointValue
    variable = T
    index = 4
    execute_on = "timestep_end"
    height = 0.87
  []
  [T6]
    type = SubChannelPointValue
    variable = T
    index = 1
    execute_on = "timestep_end"
    height = 0.87
  []
  [T7]
    type = SubChannelPointValue
    variable = T
    index = 14
    execute_on = "timestep_end"
    height = 0.87
  []
  [T8]
    type = SubChannelPointValue
    variable = T
    index = 28
    execute_on = "timestep_end"
    height = 0.87
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

[Outputs]
  csv = true
[]

[Executioner]
  type = Steady
[]

# ################################################################################
# # A multiapp that projects data to a detailed mesh
# ################################################################################

# [MultiApps]
#   [viz]
#     type = FullSolveMultiApp
#     input_files = "3d_LBE_19.i"
#     execute_on = "timestep_end"
#   []
# []

# [Transfers]
#   [xfer]
#     type = SCMSolutionTransfer
#     to_multi_app = viz
#     variable = 'mdot SumWij P DP h T rho mu q_prime S'
#   []
# []
