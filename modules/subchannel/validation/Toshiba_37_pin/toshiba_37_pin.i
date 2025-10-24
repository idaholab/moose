T_in = 660
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 37.00 / 36000.*0.5}'
P_out = 2.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = 4
    n_cells = 20
    flat_to_flat = 0.085
    heated_length = 1.0
    pin_diameter = 0.01
    pitch = 0.012
    dwire = 0.002
    hwire = 0.0833
    spacer_z = '0 0.2 0.4 0.6 0.8'
    spacer_k = '0.1 0.1 0.1 0.1 0.10'
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
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = 2.0e5
  CT = 1.0
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-6
  T_tol = 1.0e-3
  implicit = true
  segregated = false
  staggered_pressure = false
  verbose_multiapps = true
  verbose_subchannel = false
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
    power = 1.000e5 # W
    filename = "pin_power_profile_37.txt"
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
  [T_Planar_Mean]
    type = SCMPlanarMean
    variable = T
    execute_on = 'TIMESTEP_END'
    height = 1.0
  []

  [DP_SubchannelDelta]
    type = SubChannelDelta
    variable = P
    execute_on = 'TIMESTEP_END'
  []
[]
################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = "toshiba_37_pin_viz.i"
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
